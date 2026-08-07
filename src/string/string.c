#include "string.h"


int strlen(const char* ptr)
{
    int counter = 0;

    while(ptr[counter] != '\0')
    {
        counter++;
    }

    return counter;
}

bool isdigit(char c)
{
    return (c >= 48 && c < 58);
}

int chartoint(char c)
{
    return ((int)c - 48);
}

int strnlen(const char* ptr,int max)
{
    int counter = 0;

    for(int i=0;i<max;i++)
    {
        if(ptr[counter] == '\0') break;
        counter++;
    }

    return counter;
}