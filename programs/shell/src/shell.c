#include "shell.h"
#include "stdio.h"
#include "stdlib.h"
#include "os.h"

int main(int argc, char** argv)
{
    print("Welcome!!!\n");
    while(1) 
    {
        print("> ");
        char buff[1024];
        read_terminal_line(buff,sizeof(buff),true);
        low_level_process_load(buff);
        print("\n");
    } 
    return 0;
}