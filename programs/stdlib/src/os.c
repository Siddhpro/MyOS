#include "os.h"
#include "stdio.h"
#include "string.h"

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

struct arguments* parse_command(const char* ptr, int max)
{
    struct arguments* start = 0;
    struct arguments* current = 0;

    char command[1024];

    if (!ptr || max < 0 || max >= (int)sizeof(command))
    {
        return 0;
    }

    strncpy(command, ptr, max + 1);

    char* token = strtok(command, " ");

    if (!token)
    {
        goto out;
    }

    start = (struct arguments*)low_level_malloc(sizeof(struct arguments));

    if (!start)
    {
        goto out;
    }

    strncpy(start->argument, token, sizeof(start->argument));
    start->next = 0;

    current = start;

    token = strtok(0, " ");

    while (token != 0)
    {
        struct arguments* new_node = (struct arguments*)low_level_malloc(sizeof(struct arguments));

        if (!new_node)
        {
            break;
        }

        strncpy(new_node->argument, token, sizeof(new_node->argument));
        new_node->next = 0;

        current->next = new_node;
        current = new_node;

        token = strtok(0, " ");
    }

out:
    return start;
}
