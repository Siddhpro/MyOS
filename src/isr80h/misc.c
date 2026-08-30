#include "misc.h"
#include "task/task.h"

void* isr80h_command0_sum(struct interrupt_frame* frame)
{
    int a = (int)task_get_stack_item(task_current(),0);
    int b = (int)task_get_stack_item(task_current(),1);

    return (void*)(a + b); 
}