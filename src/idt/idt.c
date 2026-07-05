#include "idt.h"
#include "config.h"
#include "memory/memory.h"
#include "../kernel.h"

struct idt_desc idt_descriptors[TOTAL_INTERRUPTS];
struct idtr_desc idtr_descriptor;

extern void idt_load(void * ptr);

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

void idt_init()
{
    memset(idt_descriptors,0,sizeof(idt_descriptors));
    idtr_descriptor.limit = sizeof(idt_descriptors) - 1;
    idtr_descriptor.base = (uint32_t) idt_descriptors;

    idt_set(0,idt_zero);
    idt_load(&idtr_descriptor);
}
