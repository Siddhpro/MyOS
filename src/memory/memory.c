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

int memcmp(void *s1,void* s2,int len)
{
    unsigned char *start1 = (unsigned char*) s1;
    unsigned char *start2 = (unsigned char*) s2;

    for(int i=0;i<len;i++)
    {
        if(start1[i]!= start2[i])
        {
            if(start1[i] < start2[i]) return -1;
            if(start1[i] > start2[i]) return 1;
        }
    }

    return 0;
}
