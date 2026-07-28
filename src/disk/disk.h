#ifndef DISK_H
#define DISH_H

typedef unsigned int DISK_TPYE;

#define DISK_TYPE_REAL 0

struct disk
{
    DISK_TPYE type;
    int sector_size;
};

int disk_read_sector(int lba,int total, void* buff); 
void disk_search_and_init();
struct disk* disk_get(int index);
int disk_read_block(struct disk* idisk,unsigned int lba,int total,void *buff);


#endif