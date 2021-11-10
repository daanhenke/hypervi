#pragma once

#include "freestanding/types.h"

typedef void* efi_handle;
typedef wchar_t efi_char16;

typedef struct
{
    u64 signature;
    u32 revision;
    u32 header_size;
    u32 crc32;
    u32 reserved;
} efi_table_hdr;

typedef struct
{
    u32 type;
    void* physical_start;
    void* virtual_start;
    u64 number_of_pages;
    u64 attribute;
} efi_mem_descriptor;