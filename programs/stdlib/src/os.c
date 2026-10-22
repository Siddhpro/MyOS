#include "os.h"
#include "stdio.h"

int getkey()
{
    int val;
    while(1)
    {
        val = low_level_getkey();
        if(val != 0x00)
        {
            break;
        }
    }
    return val;
}

void read_terminal_line(char* out, int max, bool output_while_typing)
{
    int i = 0;

    for(i;i<max-1;i++)
    {
        char key = getkey();
        if(key == 13)
        {
            break;
        }

        if(output_while_typing)
        {
            low_level_putchar(key);
        }

        if(key == 0x08 && i>=1)
        {
            out[i-1] = 0x00;
            i-=2;
            continue;
        }

        out[i] = key;

    }

    out[i] = 0x00;
}
