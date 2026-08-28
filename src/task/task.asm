[BITS 32]
section .asm
global restore_general_purpose_registers
global task_return
global user_registers

task_return:
    mov ebp,esp


    mov ebx,[ebp+4]
    push dword[ebx+44] ;push data/stack selector
    push dword[ebx+40] ;push the stack pointer
    pushf
    pop eax
    or eax,0x200 ;done to enable interrupts
    push eax

    push dword[ebx+32] ;push the code segment
    push dword[ebx+28] ;push PC

    ;setup segment registers
    mov eax,[ebx+44]
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax

    push dword[ebp+4]
    call restore_general_purpose_registers
    add esp,4
    iretd ;moving to user mode

restore_general_purpose_registers:
    push ebp
    mov ebp,esp 
    mov ebx, [ebp+8]
    mov edi,[ebx]
    mov esi,[ebx+4]
    mov ebp,[ebx+8]
    mov edx,[ebx+16]
    mov ecx,[ebx+20]
    mov eax,[ebx+24]
    mov ebx,[ebx+12]
    pop ebp
    ret

user_registers:
    mov ax,0x23
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax
    ret