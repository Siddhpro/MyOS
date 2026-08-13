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

char* strcpy(char* dest, const char *src)
{
    char *temp = dest;
    while(*src != 0)
    {
        *dest = *src;
        src++;
        dest++;
    }

    *dest = 0x00;

    return temp;
}

int strncmp(const void* s1,const void* s2,int n)
{
    const unsigned char *str1 = (const unsigned char*) s1;
    const unsigned char *str2 = (const unsigned char*) s2;

    for(int i=0;i<n;i++)
    {
        if(str1[i] != str2[i])
        {
            return str1[i] - str2[i];
        }

        if(str1[i] == '\0')
        {
            return 0;
        }
    }

    return 0;
}

int strnlen_terminator(const char* str, int max, char terminator)
{
    int count = 0;
    for(count=0; count<max; count++)
    {
        if(str[count] == '\0' || str[count] == terminator)
        {
            break;
        }
    }

    return count;
}

char tolower(char c)
{
    if(c >= 65 && c <= 90)
    {
        c += 32;
    }

    return c;
}

int istrncmp(const char* s1,const char* s2,int n)
{
    unsigned char u1,u2;
    while(n-- > 0)
    {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;

        if(u1 != u2 && tolower(u1) != tolower(u2))
        {
            return u1 - u2;
        }

        if(u1 == '\0')
            return 0;
    }

    return 0;
}