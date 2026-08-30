#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include <idt/idt.h>
#include <io/io.h>
#include <memory/heap/kheap.h>
#include <memory/paging/paging.h>
#include "memory/memory.h"
#include "disk/disk.h"
#include "fs/pparser.h"
#include "string/string.h"
#include "fs/file.h"
#include "disk/stream.h"
#include "gdt/gdt.h"
#include "config.h"
#include "task/tss.h"
#include "task/task.h"
#include "task/process.h"
#include "status.h"
#include "isr80h/isr80h.h"

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

void panic(const char* str)
{
    print(str);
    while(1){};
}


static struct paging_chunk* kernel_chunk = 0;

struct tss tss;
struct gdt gdt_real[TOTAL_GDT_SEGMENTS];
struct gdt_structured gdt_structured[TOTAL_GDT_SEGMENTS] =
{
    {.base = 0x00, .limit = 0x00, .type = 0x00},
    {.base = 0x00, .limit = 0xffffffff, .type = 0x9A},   // kernel code
    {.base = 0x00, .limit = 0xffffffff, .type = 0x92},   // kernel data
    {.base = 0x00, .limit = 0xffffffff, .type = 0xF8},   // user code
    {.base = 0x00, .limit = 0xffffffff, .type = 0xf2},    // user data
    {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9} //tss
};

void kernel_page()
{
    kernel_registers();
    paging_switch(kernel_chunk);
}

void kernel_main()
{
    //initialise terminal
    terminal_initialize();
    print("ABCFD\n");

    // load the gdt
    memset(gdt_real,0x00,sizeof(gdt_real));
    gdt_structured_to_gdt(gdt_real,gdt_structured,TOTAL_GDT_SEGMENTS);
    gdt_load(gdt_real,sizeof(gdt_real));

    // initialise heap
    kheap_init();

    // initialise fs
    fs_init();

    // search and initialise disk
    disk_search_and_init();

    // set idt
    idt_init();

    //TSS
    memset(&tss,0,sizeof(tss));
    tss.esp0 = 0x60000;
    tss.ss0 = KERNEL_DATA_SELECTOR;
    tss_load(0x28);

    // set paging
    kernel_chunk = initialise_chunk(PAGING_READ_WRITE | PAGING_ACESS_FROM_ALL | PAGING_IS_PRESENT);
    paging_switch(kernel_chunk);
    enable_paging();

    isr80h_register_commands();

    struct process* process = 0;
    int res = process_load("0:/blank.bin",&process);
    if(res != ALL_OK)
    {
        panic("oops cannot load blank.bin\n");
    }

    task_run_first_task();

    while(1) {}

}