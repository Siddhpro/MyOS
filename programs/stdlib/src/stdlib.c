#include "stdlib.h"
#include "os.h"

void* malloc(size_t size)
{
    return low_level_malloc(size);
}

void free(void* ptr)
{
    return low_level_free(ptr);
}

char* itoa(int i)
{
    static char text[12];
    int loc = 11;
    text[11] = 0;

    char neg = 1;
    if(i >=0)
    {
        neg = 0;
        i = -i;
    }

    while(i)
    {
        text[--loc] = '0' - (i % 10);
        i /= 10;
    }

    if(loc == 11)
        text[--loc] = '0';

    if(neg)
        text[--loc] = '-';

    return &text[loc];
}
