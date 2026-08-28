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
    
};

int process_load(const char* filename,struct process** process);


#endif 