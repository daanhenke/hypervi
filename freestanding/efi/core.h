#pragma once

#include "freestanding/types.h"

#define ESTR(x) const_cast<wchar_t*>(L"" x)

#define PAGE_SIZE 4096
#define NUM_PAGES(y) ((y + PAGE_SIZE - 1) / PAGE_SIZE)

typedef void* efi_handle;
typedef void* efi_event;
typedef wchar_t efi_char16;

typedef struct
{
    u32 p1;
    u16 p2;
    u16 p3;
    u8 p4[8];
} efi_guid;

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

typedef struct
{
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u8 pad1;
    u32 nanosecond;
    u16 time_zone;
    u8 daylight;
    u8 pad2;
} efi_time;
