#include "process.h"
#include "task/task.h"
#include "config.h"
#include "status.h"
#include "task/process.h"
#include "string/string.h"
#include "kernel.h"

void* isr80h_command6_process_load(struct interrupt_frame* frame)
{
    void* filename_user = task_get_stack_item(task_current(),0);
    char filename[MAX_PATH];
    int res = copy_string_from_task(task_current(),filename_user,filename,MAX_PATH);

    if(res < 0)
    {
        goto out;
    }

    char path[MAX_PATH];
    strcpy(path,"0:/");
    strcpy(path+3,filename);

    struct process* process = 0;
    res = process_load_switch(path,&process);

    if(res < 0)
    {
        goto out;
    }

    task_switch(process->task);
    task_return(&process->task->registers);

out:
    return NULL;
}

void* isr80h_command7_system(struct interrupt_frame* frame)
{   
    struct arguments* arguments = task_virtual_address_to_physical(task_current(),task_get_stack_item(task_current(),0));
    if(!arguments || strlen(arguments[0].argument) == 0) //please have a look at this in case things go south
    {
        return ERROR(-EINVARG);
    }

    struct arguments* start = &arguments[0];
    const char* program_name = start->argument;

    char path[MAX_PATH];
    strcpy(path,"0:/");
    strncpy(path+3,program_name,sizeof(path));

    struct process* process = 0;
    int res =  process_load_switch(path,&process);
    if(res < 0)
    {
        return ERROR(res);
    }
    
    res = process_put_arguments(process,start);
    if(res < 0)
    {
        return ERROR(res);
    }

    task_switch(process->task);
    task_return(&process->task->registers);

    return 0;
}

void* isr80h_command8_get_arguments(struct interrupt_frame* frame)
{
    struct process* process = task_current()->process;
    struct process_arguments* arguments = task_virtual_address_to_physical(task_current(),task_get_stack_item(task_current(),0));

    process_get_arguments(process,&arguments->argc,&arguments->argv);

    return 0;
}