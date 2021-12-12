#pragma once

#include "freestanding/types.h"

enum class elf_section_type : u32
{
    null,
    progbits,
    symtab,
    strtab,
    rela,
    hash,
    dynamic,
    note,
    nobits,
    rel,
    shlib,
    dynsym
};

typedef struct
{
    size_t r_offset;
    size_t r_info;
    ssize_t r_addend;
} elf64_rela;

typedef struct
{
    u32 st_name;
    u8 st_info;
    u8 st_other;
    u16 st_shndx;
    size_t st_value;
    size_t st_size;
} elf64_sym;

enum class elf64_r_type : u32
{
    x86_64_32 = 1,
    x86_64_pc32,
    x86_64_copy = 5,
    x86_64_glob_dat,
    x86_64_junp_slot,
    x86_64_relative
};


constexpr u32 PT_NULL         = 0;
constexpr u32 PT_LOAD         = 1;
constexpr u32 PT_DYNAMIC      = 2;
constexpr u32 PT_INTERP       = 3;
constexpr u32 PT_NOTE         = 4;
constexpr u32 PT_SHLIB        = 5;
constexpr u32 PT_PHDR         = 6;
constexpr u32 PT_TLS          = 7;

#define elf64_r_get_type(x) static_cast<elf64_r_type>(x)
#define elf64_r_get_sym(x) static_cast<u32>(x >> 32)

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
    elf_section_type sh_type;
    size_t sh_flags;
    size_t sh_addr;
    size_t sh_offset;
    size_t sh_size;
    u32 sh_link;
    u32 sh_info;
    size_t sh_addralign;
    size_t sh_entsize;
} elf64_shdr;
