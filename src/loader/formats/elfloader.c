#include "elfloader.h"
#include "fs/file.h"
#include "status.h"
#include <stdbool.h>
#include "memory/memory.h"
#include "memory/heap/kheap.h"
#include "string/string.h"
#include "memory/paging/paging.h"
#include "kernel.h"
#include "config.h"

const char elf_signature[] = {0x7f,'E','L','F'};

static bool elf_valid_signature(void* buffer)
{
    return memcmp(buffer,(void*)elf_signature,sizeof(elf_signature)) == 0;
}

static bool elf_valid_class(struct elf_header* header)
{
    return header->e_ident[EI_CLASS] == ELFCLASSNONE || header->e_ident[EI_CLASS] == ELFCLASS32;
}

static bool elf_valid_encoding(struct elf_header* header)
{
    return header->e_ident[EI_DATA] == ELFDATANONE || header->e_ident[EI_DATA] == ELFDATA2LSB;
}

static bool elf_is_executable(struct elf_header* header)
{
    return header->e_type == ET_EXEC && header->e_entry >= PROGRAM_VIRTUAL_ADDRESS;
}

static bool elf_has_program_header(struct elf_header* header)
{
    return header->e_phoff != 0;
}

void* elf_memory(struct elf_file* file)
{
    return file->elf_memory;
}

struct elf_header* elf_header(struct elf_file* file)
{
    return (struct elf_header*)file->elf_memory;
}

struct elf32_shdr* elf_shder(struct elf_header* header)
{
    return (struct elf32_shdr*)((unsigned char*)header + header->e_shoff);
}

struct elf32_shdr* elf_shdr_by_index(struct elf_header* header,int index)
{
    if(header->e_shnnum <= index)
    {
        return NULL;
    }

    struct elf32_shdr* first = elf_shder(header);
    return &first[index];
}

struct elf32_phdr* elf_phdr(struct elf_header* header)
{
    if(header->e_phoff == 0)
    {
        return 0;
    }
    return (struct elf32_phdr*)((unsigned char*)header + header->e_phoff);
}

struct elf32_phdr* elf_phdr_by_index(struct elf_header* header,int index)
{
    if(index >= header->e_phnum)
    {
        return NULL;
    }
    struct elf32_phdr* first = elf_phdr(header);
    return &first[index];
}

struct elf32_shdr* elf_section(struct elf_header* header, int index)
{
    return &elf_shder(header)[index];
}

char* elf_str_table(struct elf_header* header)
{
    return (char*)header + elf_section(header,header->e_shstrndx)->sh_offset;
}

void* elf_virtual_base(struct elf_file* file)
{
    return file->virtual_base_address;
}

void* elf_virtual_end(struct elf_file* file)
{
    return file->virtual_end_address;
}

void* elf_physical_base(struct elf_file* file)
{
    return file->physical_base_address;
}

void* elf_physical_end(struct elf_file* file)
{
    return file->physical_end_address;
}

int elf_validate_loader(struct elf_header* header)
{
    return (elf_valid_signature(header) && elf_valid_class(header) && elf_valid_encoding(header) && elf_has_program_header(header)) ? ALL_OK : -EINFORMAT;
}

int elf_process_phdr_pt_load(struct elf_file* file,struct elf32_phdr* phdr)
{
    if(file->virtual_base_address >= (void*) phdr->p_vaddr || file->virtual_base_address == 0x00)
    {
        file->virtual_base_address = (void*) phdr->p_vaddr;
        file->physical_base_address = elf_memory(file) + phdr->p_offset;
    }

    unsigned int end_virtual_address = phdr->p_vaddr + phdr->p_filesz;

    if(file->virtual_end_address <= (void*)(end_virtual_address) || file->virtual_end_address == 0x00)
    {
        file->virtual_end_address = (void*) end_virtual_address;
        file->physical_end_address = elf_memory(file)+phdr->p_offset + phdr->p_filesz;
    }

    return 0;
}

int elf_process_pheader(struct elf_file* file,struct elf32_phdr* phdr)
{
    int res = 0;
    switch(phdr->p_type)
    {
        case PT_LOAD:
            res = elf_process_phdr_pt_load(file,phdr);
        break;
    }

    return res;
}

int elf_process_pheaders(struct elf_file* file)
{
    int res = 0;
    struct elf_header* header = elf_header(file);

    for(int i=0;i<header->e_phnum;i++)
    {
        struct elf32_phdr* pheader = elf_phdr_by_index(header,i);
        res = elf_process_pheader(file,pheader);

        if(res < 0)
        {
            goto out;
        }
    }


out:
    return res;
}

int elf_process_loaded(struct elf_file* file)
{
    int res = 0;
    struct elf_header* header = elf_header(file);

    res = elf_validate_loader(header);
    if(res < 0)
    {
        goto out;
    }

    res = elf_process_pheaders(file);
    if(res < 0)
    {
        goto out;
    }

out:
    return res;
}

int elf_load(const char* filename,struct elf_file** fileout)
{
    struct elf_file* elf_file = kzalloc(sizeof(struct elf_file));
    int fd = 0;
    int res = fopen(filename, "r");
    if(res <=0 )
    {
        goto out;
    }

    fd = res;

    struct file_stat stat;
    res = fstat(fd,&stat);
    if(res < 0)
    {
        goto out;
    }

    elf_file->elf_memory = kzalloc(stat.filesize);
    res = fread(elf_file->elf_memory,stat.filesize,1,fd);
    if(res < 0)
    {
        goto out;
    }

    res = elf_process_loaded(elf_file);
    if(res < 0)
    {
        goto out;
    }

    *fileout = elf_file; 

out:
    fclose(fd);
    return res;
}

void elf_close(struct elf_file* file)
{
    if(!file)
    {
        return;
    }

    kfree(file->elf_memory);
    kfree(file);

    return;
}