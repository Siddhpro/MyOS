#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PAGING_CACHE_DISABLED 0b00010000
#define PAGING_WRITE_THROUGH  0b00001000
#define PAGING_ACESS_FROM_ALL 0b00000100
#define PAGING_READ_WRITE     0b00000010
#define PAGING_IS_PRESENT     0b00000001

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096


struct paging_chunk
{
    uint32_t *directory;
};


struct paging_chunk* initialise_chunk(uint8_t flags);
uint32_t* get_directory_from_chunk(struct paging_chunk* chunk);
void paging_switch(struct paging_chunk* directory);
void enable_paging();
void paging_free(struct paging_chunk* chunk);

int paging_set(uint32_t* directory, void* virtual_address, uint32_t val);
bool is_aligned(void *address);
void* paging_align_address(void* ptr);

int paging_map_to(struct paging_chunk* directory, void* virtual_addr, void* physical, void* physical_end,int flags);
int paging_map_range(struct paging_chunk* directory,void* virtual_addr,void* physical,int count,int flags);
int paging_map(struct paging_chunk* directory,void* virtual_addr,void* physical,int flags);


#endif