#include "paging.h"
#include "../heap/kheap.h"
#include "../../status.h"

extern void paging_load_directory(uint32_t* directory);
extern void enable_paging();

static uint32_t* current_directory = 0;

struct paging_chunk* initialise_chunk(uint8_t flags)
{
    uint32_t* directory = (uint32_t*)kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);

    int offset = 0;
    for(int i=0;i<PAGING_TOTAL_ENTRIES_PER_TABLE;i++)
    {
        uint32_t* entry = (uint32_t*)kzalloc(PAGING_TOTAL_ENTRIES_PER_TABLE*sizeof(uint32_t));
        for(int j=0;j<PAGING_TOTAL_ENTRIES_PER_TABLE;j++)
        {
            entry[j] = (offset + (j*PAGING_PAGE_SIZE)) | flags; 
        }

        directory[i] = (uint32_t)entry | flags | PAGING_READ_WRITE;
        offset += PAGING_PAGE_SIZE * PAGING_TOTAL_ENTRIES_PER_TABLE;
    }

    struct paging_chunk* chunk = (struct paging_chunk*)kzalloc(sizeof(struct paging_chunk));
    chunk->directory = directory;

    return chunk; 
}

uint32_t* get_directory_from_chunk(struct paging_chunk* chunk)
{
    return chunk->directory;
}

void paging_switch(struct paging_chunk* directory)
{
    paging_load_directory(directory->directory);
    current_directory = directory->directory;
}


bool is_aligned(void *address)
{
    return ((uint32_t)address % PAGING_PAGE_SIZE) == 0;
}

int paging_get_indexes(void* virtual_address, uint32_t* directory_index, uint32_t* table_index)
{
    int res = 0;

    if(!is_aligned(virtual_address))
    {
        res = -EINVARG;
        goto out;
    }

    *directory_index = ((uint32_t)virtual_address >> 22) & 0x3FF;
    *table_index = ((uint32_t)virtual_address >> 12) & 0x3FF;

out:
    return res;
}

int paging_set(uint32_t* directory, void* virtual_address, uint32_t val)
{
    int res = 0;
    if(!(is_aligned(virtual_address)))
    {
        res = -EINVARG;
        goto out;
    }

    uint32_t directory_index,table_index;
    res = paging_get_indexes(virtual_address,&directory_index,&table_index);
    if(res < 0) goto out;

    if(!(directory[directory_index] & PAGING_IS_PRESENT))
    {
        res = -EINVARG;
        goto out;
    }

    uint32_t* page_table = (uint32_t*)(directory[directory_index] & 0xFFFFF000);
    page_table[table_index] = val;

out:
    return res;
}

void paging_free(struct paging_chunk* chunk)
{
    for(int i=0;i<PAGING_TOTAL_ENTRIES_PER_TABLE;i++)
    {
        uint32_t entry = chunk->directory[i];
        uint32_t* table = (uint32_t*)(entry & 0xfffff000);
        kfree(table);
    }

    kfree(chunk->directory);
    kfree(chunk);
}

void* paging_align_address(void* ptr)
{
    if((uint32_t)ptr % PAGING_PAGE_SIZE)
    {
        return (void*)((uint32_t)ptr + PAGING_PAGE_SIZE - ((uint32_t)ptr % PAGING_PAGE_SIZE));
    }

    return ptr;
}

int paging_map(struct paging_chunk* directory,void* virtual_addr,void* physical,int flags)
{
    if((unsigned int)virtual_addr % PAGING_PAGE_SIZE)
    {
        return -EINVARG;
    }

    if((unsigned int) physical % PAGING_PAGE_SIZE)
    {
        return -EINVARG;
    }

    return paging_set(directory->directory,virtual_addr,(uint32_t)physical | flags);
}

int paging_map_range(struct paging_chunk* directory,void* virtual_addr,void* physical,int count,int flags)
{
    int res = 0;
    for(int i=0;i<count;i++)
    {
        res = paging_map(directory,virtual_addr,physical,flags);
        if(res < 0)
            break;

        virtual_addr += PAGING_PAGE_SIZE;
        physical += PAGING_PAGE_SIZE;
    }
    return res;
}

int paging_map_to(struct paging_chunk* directory, void* virtual_addr, void* physical, void* physical_end,int flags)
{
    int res = 0;
    if((uint32_t)virtual_addr % PAGING_PAGE_SIZE != 0)
    {
        res = -EINVARG;
        goto out;
    }

    if((uint32_t)physical % PAGING_PAGE_SIZE != 0)
    {
        res = -EINVARG;
        goto out;
    }

    if((uint32_t)physical_end % PAGING_PAGE_SIZE != 0)
    {
        res = -EINVARG;
        goto out;
    }

    if((uint32_t)physical_end < (uint32_t)physical)
    {
        res = -EINVARG;
        goto out;
    }

    uint32_t total_pages = (physical_end - physical)/PAGING_PAGE_SIZE;
    res = paging_map_range(directory,virtual_addr,physical,total_pages,flags);
    if(res != ALL_OK)
    {
        res = -EIO;
        goto out;
    }

out:
    return res;
}

uint32_t paging_get(uint32_t* directory,void* virtual_address)
{
    uint32_t directory_index = 0;
    uint32_t table_index = 0;
    paging_get_indexes(virtual_address,&directory_index,&table_index);

    uint32_t entry = directory[directory_index];
    uint32_t* table = (uint32_t*)(entry & 0xfffff000);
    return table[table_index];
}   