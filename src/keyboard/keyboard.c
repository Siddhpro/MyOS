#include "keyboard.h"
#include "status.h"
#include "kernel.h"
#include "task/task.h"
#include "ps2.h"

static struct keyboard* head = 0;
static struct keyboard* tail = 0;

void keyboard_init()
{
    keyboard_insert(ps2_init());
}

int keyboard_insert(struct keyboard* kb)
{
    int res = 0;
    if(kb->init == 0)
    {
        res = -EINVARG;
        goto out;
    }

    if(tail)
    {
        tail->next = kb;
        tail = kb;        
    }

    else
    {
        head = kb;
        tail = kb;
    }

    res = kb->init();
out:
    return res;
}

int keyboard_get_tail_index(struct process* process)
{
    process->keyboard.tail = process->keyboard.tail % sizeof(process->keyboard.buffer);
    return process->keyboard.tail;
}

void keyboard_backspace(struct process* process)
{
    process->keyboard.tail--;
    int real_index = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index] = 0x00;
}

void keyboard_push(char c)
{
    struct process* process = process_current();

    if(!process)
    {
        return;
    }

    if(c == 0)
    {
        return;
    }


    int real_index = keyboard_get_tail_index(process);
    process->keyboard.buffer[real_index] = c;
    process->keyboard.tail++;
}

char keyboard_pop()
{
    if(!task_current())
    {
        return 0;
    }

    struct process* process = task_current()->process;
    int real_index = process->keyboard.head % sizeof(process->keyboard.buffer);
    char c = process->keyboard.buffer[real_index];
    if(c == 0x00)
    {
        return 0;
    }

    process->keyboard.buffer[real_index] = 0;
    process->keyboard.head++;

    return c;
}