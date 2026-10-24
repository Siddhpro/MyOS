#include "process.h"
#include "memory/memory.h"
#include "status.h"
#include "memory/heap/kheap.h"
#include "fs/file.h"
#include "string/string.h"
#include "kernel.h"
#include "memory/paging/paging.h"
#include "loader/formats/elfloader.h"

struct process* current_process = 0;
static struct process* processes[MAX_PROCESSES] = {};

static void process_init(struct process* process)
{
    memset(process,0,sizeof(struct process));
}

struct process* process_current()
{
    return current_process;
}

struct process* process_get(int pid)
{
    if(pid < 0 || pid >= MAX_PROCESSES)
    {
        return NULL;
    }

    return processes[pid];
}

int find_allocation_index(struct process* process)
{
    int res = -1;
    for(int i=0;i<MAX_PROGRAM_ALLOCATIONS;i++)
    {
        if(process->allocations[i] == 0x00)
        {
            res = i;
            break;
        }
    }

    if(res == -1)
    {
        res = -ENOMEM;
    }

    return res;
}

void* process_malloc(struct process* process,size_t size)
{
    void* ptr = kzalloc(size);
    if(!ptr)
    {
        goto out;
    }

    int index = find_allocation_index(process);
    if(index < 0)
    {
        goto out;
    }

    int res = paging_map_to(process->task->page_directory,ptr,ptr,paging_align_address(ptr+size),PAGING_IS_PRESENT | PAGING_READ_WRITE | PAGING_ACESS_FROM_ALL);

    if(res < 0)
    {
        goto out;
    }
    process->allocations[index] = ptr;
    process->malloc_sizes[index] = size;

    return ptr;

out:
    if(ptr)
    {
        kfree(ptr);
    }

    return 0;
}

int find_ptr_index(struct process* process, void* ptr)
{
    int index = -1;
    for(int i=0;i<MAX_PROGRAM_ALLOCATIONS;i++)
    {
        if((uint32_t)ptr == (uint32_t)process->allocations[i])
        {
            index = i;
            break;
        }
    }

    return index;
}

void process_free(struct process* process,void* ptr)
{
    int index = find_ptr_index(process,ptr);

    if(index == -1)
    {
        return;
    }

    int res = paging_map_to(process->task->page_directory,ptr,ptr,paging_align_address(ptr+process->malloc_sizes[index]),0x00);
    if(res < 0)
    {
        return;
    }
    kfree(ptr);

    process->allocations[index] = 0x00;
    process->malloc_sizes[index] = 0;
}

static int process_load_binary(const char* filename,struct process* process)
{
    int res = 0;
    int fd = fopen(filename,"r");
    if(!fd)
    {
        res = -EIO;
        goto out;
    }

    struct file_stat stat;
    res = fstat(fd,&stat);
    if(res != ALL_OK)
    {
        goto out;
    }

    void* program_data_ptr = kzalloc(stat.filesize);
    if(!program_data_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    if(fread(program_data_ptr,stat.filesize,1,fd) != 1)
    {
        res = -EIO;
        goto out;
    }

    process->ptr = program_data_ptr;
    process->size = stat.filesize;
    process->filetype = PROCESS_FILETYPE_BINARY;

out:
    fclose(fd);
    return res;
}

static int process_load_elf(const char* filename,struct process* process)
{
    int res = 0;
    struct elf_file* file = 0;
    res = elf_load(filename,&file);

    if(ISERR(res))
    {
        goto out;
        res = -EIO;
    }

    process->filetype = PROCESS_FILETYPE_ELF;
    process->elf_file = file;

out:
    return res;
}

static int process_load_data(const char* filename,struct process* process)
{
    int res = 0;
    res = process_load_elf(filename,process);
    if(res == -EINFORMAT)
        res = process_load_binary(filename,process);
    return res;
}


int process_map_binary(struct process* process)
{
    int res = 0;
    paging_map_to(process->task->page_directory, (void*)PROGRAM_VIRTUAL_ADDRESS,process->ptr,paging_align_address(process->ptr + process->size),PAGING_IS_PRESENT | PAGING_ACESS_FROM_ALL | PAGING_READ_WRITE);
    return res;
}

int process_map_elf(struct process* process)
{
    int res = 0;
    struct elf_file* file = process->elf_file;

    struct elf_header* header = elf_header(file);
    
    for(int i=0;i<header->e_phnum;i++)
    {
        struct elf32_phdr* phdr = elf_phdr_by_index(header,i);
        void* data_address = (void*)((uint32_t)header + phdr->p_offset);
        int flags = PAGING_IS_PRESENT | PAGING_ACESS_FROM_ALL;
        if(phdr->p_flags & PF_W) 
        {
            flags |= PAGING_READ_WRITE;
        }

        res = paging_map_to(process->task->page_directory,paging_align_to_lower_page((void*)phdr->p_vaddr),paging_align_to_lower_page(data_address),paging_align_address(paging_align_address(data_address + phdr->p_memsz)),flags);
    
        if(ISERR(res))
        {
            break;
        }
    }



out:
    return res;
}

int process_map_memory(struct process* process)
{
    int res = 0;

    switch(process->filetype)
    {
        case PROCESS_FILETYPE_BINARY:
            res = process_map_binary(process);
            break;

        case PROCESS_FILETYPE_ELF:
            res = process_map_elf(process);
            break;

        default:
            panic("Can't process filetype");
    }

    if(res < 0)
    {
        goto out;
    }

    paging_map_to(process->task->page_directory,(void*)PROGRAM_VIRTUAL_STACK_ADDRESS_END,process->stack,paging_align_address(process->stack + USER_PROGRAM_STACK_SIZE),PAGING_IS_PRESENT|PAGING_ACESS_FROM_ALL|PAGING_READ_WRITE);
out:
    return res;
}


int process_load_for_slot(const char* filename, struct process** process, int process_slot)
{
    int res  = 0;
    struct task* task = 0;
    struct process* _process;
    void* program_stack_ptr = 0;

    if(process_get(process_slot) != 0)
    {
        res = -EISTKN;
        goto out;
    }

    _process = kzalloc(sizeof(struct process));
    if(!_process)
    {
        res = -ENOMEM;
        goto out;
    }

    process_init(_process);
    res = process_load_data(filename,_process);
    if(res < 0)
    {
        goto out;
    }

    program_stack_ptr = kzalloc(USER_PROGRAM_STACK_SIZE);
    if(!program_stack_ptr)
    {
        res = -ENOMEM;
        goto out;
    }

    strncpy(_process->filename,filename,sizeof(_process->filename));
    _process->stack = program_stack_ptr;
    _process->id = process_slot;

    task = task_new(_process);
    if(ERROR_I(task) == 0)
    {
        res = ERROR_I(task);
        goto out;
    }
    _process->task = task;

    res = process_map_memory(_process);
    if(res != ALL_OK)
    {
        goto out;
    }

    *process = _process;

    processes[process_slot] = _process;

out:
    if(ISERR(res))
    {
        if(_process && _process->task)
        {
            task_free(_process->task);
        }

        // free the process data
    }
    return res;
}

int process_get_free_slot()
{
    for(int i=0;i<MAX_PROCESSES;i++)
    {
        if(processes[i] == 0)
        {
            return i;
        }
    }

    return -EISTKN;
}

int process_load(const char* filename,struct process** process)
{
    int res = 0;
    int process_slot = process_get_free_slot();
    if(process_slot < 0)
    {
        res = -EISTKN;
        goto out;
    }

    res = process_load_for_slot(filename,process,process_slot);

out:
    return res;
}

int process_load_switch(const char* filename,struct process** process)
{
    int res = process_load(filename,process);
    if(res == 0)
    {
        process_switch(*process);
    }

    return res;
}

int process_switch(struct process* process)
{
    current_process = process;
    return 0;
}

void process_get_arguments(struct process* process, int* argc, char*** argv)
{
    *argc = process->arguments.argc;
    *argv = process->arguments.argv;
}

int process_put_arguments(struct process* process,struct arguments* start)
{
    int res = 0;
    struct arguments* current = start;
    int i = 0;

    int argc = 0;
    while(current)
    {
        argc++;
        current = current->next;
    }
    current = start;

    if(argc == 0)
    {
        res = -EIO;
        goto out;
    }

    char **argv = process_malloc(process,sizeof(const char*) * argc);
    if(!argv)
    {
        res = -ENOMEM;
        goto out;
    }

    while(current)
    {
        char* arg = process_malloc(process,sizeof(current->argument));
        if(!arg)
        {
            res = -ENOMEM;
            goto out;
        }
        strncpy(arg,current->argument,sizeof(current->argument));

        argv[i] = arg;
        i++;
        current = current->next;
    }

    process->arguments.argc = argc;
    process->arguments.argv = argv;

out:
    return res;
}

int malloc_terminate(struct process* process)
{
    for(int i=0;i<MAX_PROGRAM_ALLOCATIONS;i++)
    {
        if(process->allocations[i] != 0x00)
        {
            process_free(process,process->allocations[i]);
        }
    }

    return 0;
}

int free_data(struct process* process)
{
    int res = 0;

    switch(process->filetype)
    {
        case PROCESS_FILETYPE_BINARY:
            kfree(process->ptr);
            break;

        case PROCESS_FILETYPE_ELF:
            elf_close(process->elf_file);
            break;

        default:
            res = -EINVARG;
    }
    return res;
}

void process_switch_to_any()
{
    for(int i=0;i<MAX_PROCESSES;i++)
    {
        if(processes[i] != 0x00)
        {
            process_switch(processes[i]);
            return;
        }
    }

    panic("No process to switch to\n");
}

void remove_from_array(struct process* process)
{
    processes[process->id] = 0x00;

    if(current_process == process)
    {
        process_switch_to_any();
        return;
    }
}

int process_terminate(struct process* process)
{
    if(process->parent)
    {
        process->parent->task->state = TASK_STATE_READY;
    }
    int res = malloc_terminate(process);
    if(res < 0)
    {
        goto out;
    }

    res = free_data(process);
    if(res < 0)
    {
        goto out;
    }

    kfree(process->stack);

    task_free(process->task);

    remove_from_array(process);

out:
    return res;
}