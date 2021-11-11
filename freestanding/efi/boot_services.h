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

enum class efi_locate_search_type : u64
{
    all_handles,
    by_register_notify,
    by_protocol
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

    u64 padding[11];

    efi_status (*handle_protocol)(efi_handle handle, efi_guid* protocol, void** interface);
    void* reserved;

    u64 padding2[18];

    efi_status (*locate_handle_buffer)(efi_locate_search_type search_type, efi_guid* protocol, void* search_key, size_t* handle_count, efi_handle** buffer);

} efi_boot_services;
