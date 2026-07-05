#include "memory.h"

void* memset(void *ptr,int c,size_t num)
{
    char *c_ptr = (char*) c;

    for(int i=0;i<num;i++)
    {
        c_ptr[i] = c;
    }

    return ptr;
}