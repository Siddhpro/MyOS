#include "os.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc,char** argv)
{
    struct process_arguments arguments;
    low_level_get_arguments(&arguments);

    printf("%d %s",arguments.argc,arguments.argv[0]);
    while(1){};
}