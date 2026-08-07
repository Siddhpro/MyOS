#include "pparser.h"
#include "string/string.h"
#include "kernel.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "status.h"

static int pathparser_valid_format(const char* path)
{
    int len = strnlen(path, MAX_PATH);
    return (len >= 3 &&
            isdigit(path[0]) &&
            memcmp((void*)&path[1], ":/", 2) == 0);
}

static int pathparser_get_drive(const char** path)
{
    if (!pathparser_valid_format(*path))
    {
        return -EBADPATH;
    }

    int drive = chartoint((*path)[0]);

    *path += 3;

    return drive;
}

static struct path_root* pathparser_create_root(int drive_number)
{
    struct path_root* root = kzalloc(sizeof(struct path_root));

    if (!root)
    {
        return 0;
    }

    root->drive_number = drive_number;
    root->first = 0;

    return root;
}

static const char* pathparser_get_path_part(const char** path)
{
    int i = 0;
    char* path_part_name = kzalloc(MAX_PATH);

    if (!path_part_name)
    {
        return 0;
    }

    while ((*path)[i] != '/' && (*path)[i] != '\0')
    {
        path_part_name[i] = (*path)[i];
        i++;
    }

    path_part_name[i] = '\0';

    *path += i;

    if (**path == '/')
    {
        (*path)++;
    }

    if (i == 0)
    {
        kfree(path_part_name);
        return 0;
    }

    return path_part_name;
}

struct path_part* pathparser_parse_path_part(struct path_part* last,const char** path)
{
    const char* path_part_str = pathparser_get_path_part(path);

    if (!path_part_str)
        return 0;

    struct path_part* new_path_part = kzalloc(sizeof(struct path_part));

    if (!new_path_part)
    {
        kfree((void*)path_part_str);
        return 0;
    }

    new_path_part->part = path_part_str;
    new_path_part->next = 0;

    if (last)
    {
        last->next = new_path_part;
    }

    return new_path_part;
}

void pathparser_free(struct path_root* root)
{
    if (!root)
        return;

    struct path_part* current = root->first;

    while (current)
    {
        struct path_part* next = current->next;
        kfree((void*)current->part);
        kfree(current);
        current = next;
    }

    kfree(root);
}

// current_directory_path will be used for relative paths
struct path_root* pathparser_parse(const char* path,const char* current_directory_path)
{
    int res = 0;
    const char* temp_path = path;
    struct path_root* path_root = 0;

    (void)current_directory_path;

    if (!path)
    {
        goto out;
    }

    if (strlen(path) > MAX_PATH)
    {
        goto out;
    }

    res = pathparser_get_drive(&temp_path);
    if (res < 0)
    {
        goto out;
    }

    path_root = pathparser_create_root(res);
    if (!path_root)
    {
        goto out;
    }

    struct path_part* first_part = pathparser_parse_path_part(NULL, &temp_path);

    if (!first_part)
    {
        pathparser_free(path_root);
        path_root = 0;
        goto out;
    }

    path_root->first = first_part;

    struct path_part* current = first_part;

    while (1)
    {
        struct path_part* next = pathparser_parse_path_part(current, &temp_path);

        if (!next)
        {
            break;
        }

        current = next;
    }

out:
    return path_root;
}