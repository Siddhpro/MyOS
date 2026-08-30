#include "task.h"
#include "kernel.h"
#include "status.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "process.h"
#include "string/string.h"

struct task* current_task = 0;
struct task* task_tail = 0;
struct task* task_head = 0;

struct task* task_current()
{
    return current_task;
}


struct task* task_get_next()
{
    if(!current_task->next)
    {
        return task_head;
    }

    return current_task->next;
}

static void task_list_remove(struct task* task)
{
    if(task->prev)
    {
        task->prev->next = task->next;
    }

    if(task->next)
    {
        task->next->prev = task->prev;
    }

    if(task == task_head)
    {
        task_head = task->next;
    }

    if(task == task_tail)
    {
        task_tail = task->prev;
    }

    if(task == current_task)
    {
        current_task = task_get_next();
    }
}

int task_free(struct task* task)
{
    paging_free(task->page_directory);
    task_list_remove(task);

    kfree(task);
    return 0;
}

int task_init(struct task* task,struct process* process)
{
    memset(task,0,sizeof(struct task));
    task->page_directory = initialise_chunk(PAGING_IS_PRESENT | PAGING_ACESS_FROM_ALL);
    if(!task->page_directory)
    {
        return -EIO;
    }
    
    task->registers.ip = PROGRAM_VIRTUAL_ADDRESS;
    task->registers.ss = USER_DATA_SEGMENT;
    task->registers.cs = USER_CODE_SEGMENT;
    task->registers.esp = PROGRAM_VIRTUAL_STACK_ADDRESS_START;
    task->process = process;

    return 0;
}

struct task* task_new(struct process* process)
{
    int res = 0;
    struct task* task = kzalloc(sizeof(struct task));
    if(!task)
    {
        res = -ENOMEM;
        goto out;
    }

    res = task_init(task,process);
    if(res != ALL_OK)
    {
        goto out;
    }

    if(!task_head)
    {
        task_head = task;
        task_tail = task;
        current_task = task;
        goto out;
    }

    task_tail->next = task;
    task->prev = task_tail;
    task_tail = task;


out:
    if(ISERR(res))
    {
        task_free(task);
        return ERROR(res);
    }

    return task;

}

int task_page_task(struct task* task)
{
    user_registers();
    task_switch(task);
    return 0;
}

int task_page()
{
    user_registers();
    task_switch(current_task);
    return 0;
}

int task_switch(struct task* task)
{
    current_task = task;
    paging_switch(task->page_directory);
    return 0;
}

void task_run_first_task()
{
    if(!current_task)
    {
        panic("task_run_first_task():no task available!");
    }

    task_switch(task_head);
    task_return(&task_head->registers);
}

int copy_string_from_task(struct task* task,void* virtual,void* physical,int max)
{
    if(max >= PAGING_PAGE_SIZE)
    {
        return -EINVARG;
    }

    int res = 0;
    char* temp = kzalloc(max);
    if(!temp)
    {
        res = -ENOMEM;
        goto out;
    }

    uint32_t* task_directory = task->page_directory->directory;
    uint32_t old_entry = paging_get(task_directory,temp);

    paging_map(task->page_directory,temp,temp,PAGING_READ_WRITE|PAGING_IS_PRESENT|PAGING_ACESS_FROM_ALL);
    paging_switch(task->page_directory);
    strncpy(temp,virtual,max);
    kernel_page();

    res = paging_set(task_directory,temp,old_entry);
    if(res < 0)
    {
        res = -EIO;
        goto out_free;
    }

    strncpy(physical,temp,max);

out_free:
    kfree(temp);

out:
    return res;
}

void task_save_state(struct task* task, struct interrupt_frame* frame)
{
    task->registers.ip = frame->ip;
    task->registers.cs = frame->cs;
    task->registers.flags = frame->flags;
    task->registers.esp = frame->esp;
    task->registers.ss = frame->ss;
    task->registers.eax = frame->eax;
    task->registers.ebp = frame->ebp;
    task->registers.ebx = frame->ebx;
    task->registers.ecx = frame->ecx;
    task->registers.edi = frame->edi;
    task->registers.edx = frame->edx;
    task->registers.esi = frame->esi;
}

void task_current_save_state(struct interrupt_frame* frame)
{
    if(!task_current())
    {
        panic("task_current_save_state: no current task to save\n");
    }

    struct task* task = task_current();
    task_save_state(task,frame);
}

void* task_get_stack_item(struct task* task,int index)
{
    void* res = 0;
    uint32_t* sp = (uint32_t*)task->registers.esp;

    task_page_task(task);
    res  = (void*) sp[index];

    kernel_page();
    return res;
}