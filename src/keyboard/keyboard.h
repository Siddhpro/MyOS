#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "task/process.h"

typedef int (*KEYBOARD_INIT_FUNCTION)();

#define KEYBOARD_CAPS_LOCK_ON 1
#define KEYBOARD_CAPS_LOCK_OFF 0
typedef int KEYBOARD_CAPS_LOCK;


struct keyboard
{
    KEYBOARD_INIT_FUNCTION init;
    char name[20];
    struct keyboard* next;

    KEYBOARD_CAPS_LOCK capslock;
};

void keyboard_init();
void keyboard_backspace(struct process* process);
void keyboard_push(char c);
char keyboard_pop();
int keyboard_insert(struct keyboard* kb);
void keyboard_set_capslock(struct keyboard* keyboard,KEYBOARD_CAPS_LOCK state);
KEYBOARD_CAPS_LOCK keyboard_get_capslock(struct keyboard* keyboard);


#endif 