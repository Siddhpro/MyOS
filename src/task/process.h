#ifndef PROCESS_H
#define PROCESS_H
#include <stdint.h>
#include "config.h"
#include "task.h"

struct process
{
    uint16_t id;
    char filename[MAX_PATH];

    struct task* task;
    void* allocations[MAX_PROGRAM_ALLOCATIONS]; //used for keeping track of malloc

    void* ptr; //physical pointer to process memory
    void* stack; //physical pointer to stack
    uint32_t size; // size of data pointed by ptr
    
    struct keyboard_buffer
    {
        char buffer[KEYBOARD_BUFFER_SIZE];
        int tail;
        int head;
    }keyboard;
};

int process_load(const char* filename,struct process** process);
struct process* process_current();
int process_load_switch(const char* filename,struct process** process);
int process_switch(struct process* process);


#endif 