#include "os.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

int main(int argc,char** argv)
{
    char c = getkey();
    putchar(c);
    return 0;
}