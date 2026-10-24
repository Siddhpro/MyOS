#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "../kernel.h"
#include "../io/io.h"
#include "kernel.h"
#include "task/task.h"
#include "task/process.h"
#include "status.h"

struct idt_desc idt_descriptors[TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void* interrupt_pointer_table[TOTAL_INTERRUPTS];
static INTERRUPT_CALLBACK_FUNCTION interrupt_callbacks[TOTAL_INTERRUPTS];
static ISR80H_COMMAND isr80h_commands[MAX_ISR80H_COMMANDS];

extern void idt_load(void * ptr);
extern void no_interrupt();
extern void isr80h_wrapper();

void no_interrupt_handler()
{
    outb(0x20,0x20);
}

void interrupt_handler(int interrupt,struct interrupt_frame* frame)
{
    kernel_page();
    if(interrupt_callbacks[interrupt] != 0)
    {
        task_current_save_state(frame);
        interrupt_callbacks[interrupt](frame);
    }

    task_page();
    outb(0x20,0x20);
}

void idt_zero()
{
    print("Divide by zero error\n");
}

void idt_set(int interrupt_no, void* address)
{
    struct idt_desc temp;
    temp.offset1 = (uint32_t) address & 0x0000ffff;
    temp.selector = KERNEL_CODE_SELECTOR;
    temp.zero = 0;
    temp.type_attr = 0xEE;
    temp.offset_2 = ((uint32_t) address >> 16) & 0x0000ffff;

    idt_descriptors[interrupt_no] = temp;
}

void idt_handle_exceptions()
{
    process_terminate(task_current()->process);

    task_next();
}

void idt_init()
{
    memset(idt_descriptors,0,sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t) idt_descriptors;

    for(int i=0;i<TOTAL_INTERRUPTS;i++)
    {
        idt_set(i,interrupt_pointer_table[i]);
    }
    
    for(int i=0;i<0x20;i++)
    {
        idt_register_interrupt_callback(i,idt_handle_exceptions);
    }
    
    idt_load(&idtr_descriptor);
}

void isr80h_register_command(int command, ISR80H_COMMAND func)
{
    if(command < 0 || command >= MAX_ISR80H_COMMANDS)
    {
        panic("80h interrupt command out of bounds\n");
    }

    if(isr80h_commands[command])
    {
        panic("80h interrupt command already exists\n");
    }

    isr80h_commands[command] = func;
}

int idt_register_interrupt_callback(int interrupt,INTERRUPT_CALLBACK_FUNCTION interrupt_callback)
{
    int res = 0;
    if(interrupt < 0 || interrupt >= TOTAL_INTERRUPTS)
    {
        res = -EINVARG;
        return res;
    }


    interrupt_callbacks[interrupt] = interrupt_callback;

    return res;
}


void* isr80h_handle_command(int command,struct interrupt_frame* frame)
{
    void* res = 0;

    if(command < 0 || command >= MAX_ISR80H_COMMANDS)
    {
        return 0;
    }

    ISR80H_COMMAND func = isr80h_commands[command];
    if(!func)
    {
        return 0;
    }

    res = func(frame);

    return res;
}

void* isr80h_handler(int command,struct interrupt_frame* frame)
{
    void* res = 0;
    kernel_page();
    task_current_save_state(frame);

    res = isr80h_handle_command(command,frame);

    task_page();
    return res;
}
