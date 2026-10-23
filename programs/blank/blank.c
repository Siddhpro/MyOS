#include "os.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc,char** argv)
{
    char str[] = "hello world";
    struct arguments* start = parse_command(str,sizeof(str));
    printf("%s\n",start->argument);
    printf("%s\n",start->next->argument);
    while(1){};
}