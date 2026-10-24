#ifndef ISR80H_PROCESS_H
#define ISR80H_PROCESS_H
struct interrupt_frame;

void* isr80h_command6_process_load(struct interrupt_frame* frame);
void* isr80h_command7_system(struct interrupt_frame* frame);
void* isr80h_command8_get_arguments(struct interrupt_frame* frame);
void* isr80h_command9_exit(struct interrupt_frame* frame);

#endif