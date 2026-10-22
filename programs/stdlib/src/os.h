#ifndef OS_H
#define OS_H

#include <stddef.h>
#include <stdbool.h>

void print(const char* message);
int low_level_getkey();
void low_level_putchar(char c);
void* low_level_malloc(size_t size);
void low_level_free(void* ptr);
void read_terminal_line(char* out, int max, bool output_while_typing);
int getkey();
void low_level_process_load(const char* filename);

#endif