#include "keyboard.h"
#include "ps2.h"
#include <stdint.h>
#include <stddef.h>
#include "io/io.h"
#include "kernel.h"
#include "idt/idt.h"
#include "task/task.h"

static uint8_t keyboard_scan_set[] = {
    0x00, 0x1B, '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    0x08, '\t', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    0x0d, 0x00, 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ';', '\'', '`', 
    0x00, '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', ',', '.', '/', 0x00, '*',
    0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.'
};

uint8_t ps2_scancode_to_char(uint8_t scancode);
void ps2_keyboard_handle_interrupt();
int ps2_keyboard_init()
{
    idt_register_interrupt_callback(ISR_KEYBOARD_INTERRUPT,ps2_keyboard_handle_interrupt);
    outb(PS2_PORT,PS2_ENABLE_FIRST_PORT);
    return 0;
}

struct keyboard ps2 = {
    .name = "ps2",
    .init = ps2_keyboard_init
};


struct keyboard* ps2_init()
{
    return &ps2;
}

void ps2_keyboard_handle_interrupt()
{
    kernel_page();
    uint8_t scancode = 0;

    scancode = insb(KEYBOARD_INPUT_PORT);
    insb(KEYBOARD_INPUT_PORT);

    if(scancode & PS2_KEYBOARD_KEY_RELEASED)
    {
        return;
    }

    uint8_t c = ps2_scancode_to_char(scancode);
    if(c != 0)
    {
        keyboard_push(c);
        /*
        char str[2];
        str[0] = c;
        str[1] = '\0';
        print(str);
        */
    }

    task_page();
}

uint8_t ps2_scancode_to_char(uint8_t scancode)
{
    size_t size = sizeof(keyboard_scan_set) / sizeof(uint8_t);

    if(scancode > size)
    {
        return 0;
    }

    char c = keyboard_scan_set[scancode];
    return c;
}

