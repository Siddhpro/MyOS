#include "stream.h"
#include "memory/heap/kheap.h"
#include "config.h"
#include "status.h"

struct disk_stream* diskstream_create(int disk_id)
{
    struct disk* disk = disk_get(disk_id);
    if(!disk)
    {
        return 0;
    }

    struct disk_stream* stream = kzalloc(sizeof(struct disk_stream));
    stream->position = 0;
    stream->disk = disk;

    return stream;
}

int diskstream_seek(struct disk_stream* stream, int position)
{
    stream->position = position;
    return 0;
}

int diskstream_read(struct disk_stream* stream, void* out, int total)
{
    if (total <= 0)
        return 0;

    int lba_start = stream->position / SECTOR_SIZE;
    int offset = stream->position % SECTOR_SIZE;
    int lba_end = (stream->position + total - 1) / SECTOR_SIZE;
    int lba_to_read = lba_end - lba_start + 1;

    char* buffer = kzalloc(SECTOR_SIZE);
    if (!buffer)
        return -ENOMEM;

    char* target = (char*)out;
    int target_index = 0;

    for (int sector = 0; sector < lba_to_read; sector++)
    {
        int res = disk_read_block(stream->disk,lba_start + sector,1,buffer);

        if (res < 0)
        {
            kfree(buffer);
            return res;
        }

        if (lba_to_read == 1)
        {
            for (int i = 0; i < total; i++)
            {
                target[target_index++] = buffer[offset + i];
            }
        }
        else if (sector == 0)
        {
            for (int i = offset; i < SECTOR_SIZE; i++)
            {
                target[target_index++] = buffer[i];
            }
        }
        else if (sector == lba_to_read - 1)
        {
            int last_bytes = (stream->position + total) % SECTOR_SIZE;

            if (last_bytes == 0)
                last_bytes = SECTOR_SIZE;

            for (int i = 0; i < last_bytes; i++)
            {
                target[target_index++] = buffer[i];
            }
        }
        else
        {
            for (int i = 0; i < SECTOR_SIZE; i++)
            {
                target[target_index++] = buffer[i];
            }
        }
    }

    stream->position += total;

    kfree(buffer);
    return total;
}

void diskstream_free(struct disk_stream* stream)
{
    kfree(stream);
}