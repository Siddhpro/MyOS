#include "heap.h"
#include "task/process.h"
#include <stddef.h>

void* isr80h_command4_malloc(struct interrupt_frame* frame)
{
    int size = (size_t)task_get_stack_item(task_current(),0);
    return process_malloc(task_current()->process,size);
}

void* isr80h_command5_free(struct interrupt_frame* frame)
{
    void* ptr = task_get_stack_item(task_current(),0);
    process_free(task_current()->process,ptr);
    return 0;
}