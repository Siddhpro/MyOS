#ifndef STRING_H
#define STRING_H

#include <stdbool.h>

int strlen(const char*ptr);
bool isdigit(char c);
int chartoint(char c);
int strnlen(const char* ptr,int max);
char* strcpy(char* dest, const char *src);
int strncmp(const void* s1,const void* s2,int n);
int strnlen_terminator(const char* str, int max, char terminator);
char tolower(char c);
int istrncmp(const char* s1,const char* s2,int n);
char* strncpy(char* s1,const char* s2,int size);

#endif