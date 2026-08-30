section .asm

extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler

global idt_load
global int21h
global no_interrupt
global start_interrupt
global isr80h_wrapper

start_interrupt:
    sti
    ret

clear_interrupt:
    cli
    ret

idt_load:
    push ebp;
    mov ebp,esp

    mov ebx, [ebp + 8]
    lidt[ebx]

    pop ebp
    ret


int21h:
    cli
    pushad
    call int21h_handler
    popad
    sti
    iret

no_interrupt:
    cli
    pushad
    call no_interrupt_handler
    popad
    sti
    iret

isr80h_wrapper:
    ; the processor already stores ip,cs,sp,ss,flags
    pushad ;store others
    
    push esp
    push eax ;push command
    call  isr80h_handler
    mov dword[tmp],eax
    add esp,8

    ; restore registers
    popad
    mov eax,[tmp]
    iretd

section .data
tmp: dd 0