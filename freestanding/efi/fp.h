#pragma once

#include "freestanding/efi/core.h"
#include "freestanding/efi/status.h"

typedef struct
{
    efi_char16 volume_label[128];
} efi_fp_volume_label;

typedef struct
{
    u64 size;
    u64 file_size;
    u64 physical_size;
    efi_time create_time;
    efi_time access_time;
    efi_time modification_time;
    u64 attribute;
    efi_char16 file_name[256];
} efi_fp_file_info;

enum class efi_fp_mode : u64
{
    create,
    read,
    write
};

enum class efi_fp_attribute : u64
{
    read_only = 1,
    hidden = 2,
    system = 4,
    reserved = 8,
    directory = 0x10,
    archive = 0x20,
    valid_attr = 0x37
};

struct efi_fp;
typedef struct efi_fp
{
    u64 revision;

    efi_status (*open)(efi_fp* thiz, efi_fp** new_handle, efi_char16* file_name, u64 open_mode, u64 attributes);
    efi_status (*close)(efi_fp* thiz);
    efi_status (*delete_file)(efi_fp* thiz);
    efi_status (*read)(efi_fp* thiz, size_t* buffer_size, void* buffer);

    efi_handle padding[3];

    efi_status (*get_info)(efi_fp* thiz, efi_guid* info_guid, u64* buffer_size, void* buffer);
} efi_fp;

#define efi_file_info_guid              {0x09576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}
#define efi_filesystem_info_guid        {0x09576e93, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}
#define efi_filesystem_label_guid       {0xdb47d7d3, 0xfe81, 0x11d3, {0x9a, 0x35, 0x00, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}
