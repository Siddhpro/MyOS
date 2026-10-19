[BITS 32]
section .asm
global _start

_start:
    call getkey
    push eax
    mov eax,3
    int 0x80
    add esp,4

    jmp _start

getkey:
    mov eax,2
    int 0x80
    cmp eax,0x00
    je getkey
    ret

section .data
message: db 'I am an user message!',0