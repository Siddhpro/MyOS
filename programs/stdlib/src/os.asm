[BITS 32]

section .asm

global print:function
global low_level_getkey:function
global low_level_putchar:function
global low_level_malloc:function
global low_level_free:function
global low_level_process_load:function
global low_level_get_arguments:function
global system:function

print:
    push ebp
    mov ebp,esp

    push dword[ebp + 8]
    mov eax,1
    int 0x80
    add esp,4

    pop ebp
    ret

low_level_getkey:
    push ebp
    mov ebp,esp

    mov eax,2
    int 0x80
    
    pop ebp
    ret

low_level_putchar:
    push ebp
    mov ebp,esp

    push dword[ebp + 8]
    mov eax,3
    int 0x80
    add esp,4

    pop ebp
    ret

low_level_malloc:
    push ebp
    mov ebp,esp

    push dword[ebp + 8]
    mov eax,4
    int 0x80
    add esp,4

    pop ebp
    ret

low_level_free:
    push ebp
    mov ebp,esp

    push dword[ebp + 8]
    mov eax,5
    int 0x80
    add esp,4

    pop ebp
    ret

low_level_process_load:
    push ebp
    mov ebp,esp

    push dword[ebp + 8]
    mov eax,6
    int 0x80
    add esp,4

    pop ebp
    ret

system:
    push ebp
    mov ebp,esp

    push dword[ebp + 8]
    mov eax,7
    int 0x80
    add esp,4

    pop ebp
    ret

low_level_get_arguments:
    push ebp
    mov ebp,esp

    push dword[ebp+8]
    mov eax,8
    int 0x80
    add esp,4

    pop ebp
    ret