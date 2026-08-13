#include "file.h"
#include "config.h"
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "fat/fat16.h"
#include "status.h"
#include "kernel.h"
#include "disk/disk.h"



struct filesystem* filesystems[MAX_FILESYSTEMS];
struct file_descriptor* file_descriptors[MAX_FILE_DESCRIPTORS];

static struct filesystem** fs_get_free_filesystem()
{
    for(int i=0;i<MAX_FILESYSTEMS;i++)
    {
        if(filesystems[i] == 0)
        {
            return &filesystems[i];
        }
    }

    return 0;
}

void fs_insert_filesystem(struct filesystem* filesystem)
{
    struct filesystem** fs;
    fs = fs_get_free_filesystem();

    if(!fs)
    {
        print("PANIC");
        while(1){}
    }

    *fs = filesystem;

}

static void fs_static_load()
{
    fs_insert_filesystem(fat16_init());
}

void fs_load()
{
    memset(filesystems,0,sizeof(filesystems));
    fs_static_load();
}

void fs_init()
{
    memset(file_descriptors,0,sizeof(file_descriptors));
    fs_load();
}

static int file_new_descriptor(struct file_descriptor** desc_out)
{
    int res = -ENOMEM;

    for(int i=0;i<MAX_FILE_DESCRIPTORS;i++)
    {
        if(file_descriptors[i] == 0)
        {
            struct file_descriptor* desc = kzalloc(sizeof(struct file_descriptor));
            desc->index = i+1;
            file_descriptors[i] = desc;
            *desc_out = desc;
            res = 0;
            break;
        }
    }

    return res;
}

static struct file_descriptor* file_get_descriptor(int fd)
{
    if(fd < 1 || fd >= MAX_FILE_DESCRIPTORS)
        return 0;

    int index = fd - 1;

    return file_descriptors[index];
}

struct filesystem* fs_resolve(struct disk* disk)
{
    struct filesystem* fs = 0;
    for(int i=0;i<MAX_FILESYSTEMS;i++)
    {
        if(filesystems[i] != 0 && filesystems[i]->resolve(disk) == 0)
        {
            fs = filesystems[i];
            break;
        }
    }

    return fs; 
}

FILE_MODE file_get_mode_by_string(const char* str)
{
    FILE_MODE mode = FILE_MODE_INVALID;
    if(strncmp(str,"r",1) == 0)
    {
        mode = FILE_MODE_READ;
    }

    else if(strncmp(str,"w",1) == 0)
    {
        mode = FILE_MODE_WRITE;
    }

    else if(strncmp(str,"a",1) == 0)
    {
        mode = FILE_MODE_APPEND;
    }


    return mode;
}

int fopen(const char* filename,const char* mode_string)
{
    int res = 0;

    struct path_root* path_root = pathparser_parse(filename,NULL);
    if(!path_root)
    {
        res = -EINVARG;
        goto out;
    }

    if(!path_root->first)
    {
        res = -EINVARG;
        goto out;
    }

    struct disk* disk = disk_get(path_root->drive_number);
    if(!disk)
    {
        res = -EIO;
        goto out;
    }

    if(!disk->filesystem)
    {
        res = -EIO;
        goto out;
    }

    FILE_MODE mode = file_get_mode_by_string(mode_string);
    if(mode == FILE_MODE_INVALID)
    {
        res = -EINVARG;
        goto out;
    }

    void* descriptor_private_data = disk->filesystem->open(disk,path_root->first,mode);
    if(ISERR(descriptor_private_data))
    {
        res = ERROR_I(descriptor_private_data);
        goto out;
    }

    struct file_descriptor* desc = 0;
    res = file_new_descriptor(&desc);
    if(res < 0)
    {
        goto out;
    }

    desc->filesystem = disk->filesystem;
    desc->disk = disk;
    desc->private = descriptor_private_data;

    res = desc->index;

out:
    if(res < 0)
    {
        res = 0;
    }
    return res;
}