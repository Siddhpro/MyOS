#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include <idt/idt.h>
#include <io/io.h>
#include <memory/heap/kheap.h>
#include <memory/paging/paging.h>
#include "disk/disk.h"
#include "fs/pparser.h"
#include "string/string.h"
#include "fs/file.h"
#include "disk/stream.h"

uint16_t *video_mem = 0;
uint16_t terminal_row = 0;
uint16_t terminal_column = 0;

uint16_t terminal_make_char(char c,char color)
{
    return (color << 8) | c;
}

void terminal_putchar(int x,int y,char c,char color)
{
    video_mem[y*VGA_WIDTH + x] = terminal_make_char(c,color);
}

void terminal_writechar(char c,char color)
{
    if(c == '\n')
    {
        terminal_column = 0;
        terminal_row++;
        return;
    }
    
    terminal_putchar(terminal_column,terminal_row,c,color);

    terminal_column++;

    if(terminal_column >= VGA_WIDTH)
    {
        terminal_column = 0;
        terminal_row++;
    }
}

void terminal_initialize()
{
    terminal_row = 0;
    terminal_column = 0;
    video_mem = (uint16_t*)(0xB8000);
    for(int y=0;y<VGA_HEIGHT;y++)
    {
        for(int x=0;x<VGA_WIDTH;x++)
        {
            terminal_putchar(x,y,' ',0);
        }
    }
}

void print(const char *str)
{
    size_t len = strlen(str);

    for(int i=0;i<len;i++)
    {
        terminal_writechar(*(str + i),15);
    }
}


static struct paging_chunk* kernel_chunk = 0;
void kernel_main()
{
    //initialise terminal
    terminal_initialize();
    print("ABCFD\n");

    // initialise heap
    kheap_init();

    // initialise fs
    fs_init();

    // search and initialise disk
    disk_search_and_init();

    // set idt
    idt_init();

    // set paging
    kernel_chunk = initialise_chunk(PAGING_READ_WRITE | PAGING_ACESS_FROM_ALL | PAGING_IS_PRESENT);
    paging_switch(kernel_chunk->directory);
    enable_paging();

    start_interrupt();

    int fd = fopen("0:/hello.txt","r");
    if(fd)
    {
        print("SUCCESS!\n");
    }

    while(1) {}

}