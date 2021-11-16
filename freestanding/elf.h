#pragma once

#include "freestanding/types.h"

typedef struct
{
    unsigned char e_ident[16];
    u16 e_type;
    u16 e_machine;
    u32 e_version;
    void* e_entry;
    size_t e_phoff;
    size_t e_shoff;
    u32 e_flags;
    u16 e_ehsize;
    u16 e_phentsize;
    u16 e_phnum;
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
} elf64_ehdr;

typedef struct
{
    u32 p_type;
    u32 p_flags;
    size_t p_offset;
    size_t p_vaddr;
    size_t p_paddr;
    size_t p_filesz;
    size_t p_memsz;
    size_t p_align;
} elf64_phdr;

typedef struct
{
    u32 sh_name;
    u32 sh_type;
    size_t sh_flags;
    size_t sh_addr;
    size_t sh_offset;
    size_t sh_size;
    u32 sh_link;
    u32 sh_info;
    size_t sh_addralign;
    size_t sh_entsize;
} elf64_shdr;
