#ifndef PROCESS_H
#define PROCESS_H
#include <stdint.h>
#include "config.h"
#include "task.h"


#define PROCESS_FILETYPE_ELF 0
#define PROCESS_FILETYPE_BINARY 1

typedef unsigned char PROCESS_FILETYPE;
struct process
{
    uint16_t id;
    char filename[MAX_PATH];

    struct task* task;
    void* allocations[MAX_PROGRAM_ALLOCATIONS]; //used for keeping track of malloc
    size_t malloc_sizes[MAX_PROGRAM_ALLOCATIONS]; //used for keeping track of malloc sizes

    union 
    {
        void* ptr; //physical pointer to process memory
        struct elf_file* elf_file; //pointer to elf_file
    };

    PROCESS_FILETYPE filetype;
    
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
void* process_malloc(struct process* process,size_t size);
void process_free(struct process* process,void* ptr);


#endif 