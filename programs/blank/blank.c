#include "os.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc,char** argv)
{
    char* ptr = malloc(20);
    strcpy(ptr,"hello world");
    printf("%s\n",ptr);
    free(ptr);
    
    while(1) {}
    return 0;
}