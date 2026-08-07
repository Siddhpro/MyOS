#ifndef STREAM_H
#define STREAM_H
#include "disk.h"

struct disk_stream
{
    int position;
    struct disk *disk;
};

struct disk_stream* diskstream_create(int disk_id);
int diskstream_seek(struct disk_stream* stream, int position);
int diskstream_read(struct disk_stream* stream, void* out, int total);
void diskstream_free(struct disk_stream* stream);

#endif