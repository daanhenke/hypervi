#pragma once

#include "freestanding/types.h"
#include "freestanding/efi/core.h"
#include "freestanding/efi/status.h"

enum class efi_allocate_type : u64
{
    allocate_any_pages,
    allocate_max_address,
    allocate_address
};

struct efi_boot_services;
typedef struct efi_boot_services
{
    efi_table_hdr hdr;

    efi_status (*raise_tpl)(u64 new_tpl);
    efi_status (*restore_tpl)(u64 old_tpl);

    efi_status (*allocate_pages)(efi_allocate_type alloc_type, size_t mem_type, size_t pages, void** memory);
    efi_status (*free_pages)(void* memory, size_t pages);

    efi_status (*get_memory_map)(size_t* memory_map_size, void* not_yet_implemented);
} efi_boot_services;