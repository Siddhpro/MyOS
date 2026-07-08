#include "heap.h"
#include "kernel.h"
#include "status.h"
#include "../memory.h"
#include <stdbool.h>

static int heap_validate_table(void* ptr,void* end,struct heap_table* table)
{
    int res = 0;
    size_t size = ((size_t)(end - ptr))/HEAP_BLOCK_SIZE;

    if(size != table->total)
    {
        res = -EINVARG;
        goto out;
    }

out:
    return res;
}


static int heap_validate_alignment(void *ptr)
{
    return ((unsigned int)ptr % HEAP_BLOCK_SIZE == 0);
}

int heap_create(struct heap* heap,void* ptr,void * end,struct heap_table* table)
{
    int res = ALL_OK;

    if(!heap_validate_alignment(ptr) || !heap_validate_alignment(end))
    {
        res = -EINVARG;
        goto out;
    }

    memset(heap,0,sizeof(struct heap));
    heap->start_address = ptr;
    heap->table = table;

    res = heap_validate_table(ptr,end,table);
    if(res < 0) goto out;

    size_t table_size = sizeof(HEAP_BLOCK_TABLE_ENTRY) * table->total;
    memset(table->entries,ENTRY_FREE,table_size);

out:
    return res;

}

static uint32_t blocks_needed(uint32_t val)
{
    uint32_t size = (val/HEAP_BLOCK_SIZE + (val%HEAP_BLOCK_SIZE != 0));
    return size;
}

void* heap_malloc(struct heap* heap,size_t size)
{
    uint32_t total_blocks = HEAP_SIZE_BYTES/HEAP_BLOCK_SIZE;
    uint32_t blocks = blocks_needed(size);

    void* address = heap->start_address;

    int current = 0;
    int start_index = -1;
    for(int i=0;i<total_blocks;i++)
    {
        if((heap->table->entries[i] & 0x01) != ENTRY_FREE)
        {
            current = 0;
            start_index = -1;
            continue;
        }

        if(start_index == -1)
        {
            start_index = i;
        }
        current++;

        if(current == blocks) break;
    }

    if(start_index == -1) return NULL;

    if(blocks == 1) heap->table->entries[start_index] = ENTRY_TAKEN | IS_FIRST;
    else heap->table->entries[start_index] = ENTRY_TAKEN | IS_FIRST | HAS_NEXT;

    for(int i=1;i<blocks-1;i++)
    {
        heap->table->entries[start_index + i] = ENTRY_TAKEN | HAS_NEXT;
    }

    if(blocks > 1) heap->table->entries[start_index + blocks - 1] = ENTRY_TAKEN;

    uint32_t return_address = (uint32_t)address + (start_index * HEAP_BLOCK_SIZE);
    memset((void*)return_address,0,blocks*HEAP_BLOCK_SIZE);

    return (void*) return_address;

}

void heap_free(struct heap* heap,void *ptr)
{
    int res = heap_validate_alignment(ptr);
    if(res < 0) goto out;

    uint32_t block = ((uint32_t)(ptr - heap->start_address))/HEAP_BLOCK_SIZE;

    if((heap->table->entries[block] & 0x40) != IS_FIRST)
    {
        res = EINVARG;
        goto out;
    }

    while((heap->table->entries[block] & 0x80) == HAS_NEXT)
    {
        heap->table->entries[block] = ENTRY_FREE;
        block++;
    }

    heap->table->entries[block] = ENTRY_FREE;

    return;

out:
    return;
}
