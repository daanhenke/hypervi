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

enum class efi_memory_type : size_t
{
    reserved_memory_type,
    loader_code,
    loader_data,
    boot_services_code,
    boot_services_data,
    runtime_services_code,
    runtime_services_data,
    conventional_memory,
    unusable_memory,
    acpi_reclaim_memory,
    acpi_memory_nvs,
    memory_apped_io,
    memory_mapped_io_port_space,
    pal_code,
    persistent_memory,
    max_memory_type
};

struct efi_boot_services;
typedef struct efi_boot_services
{
    efi_table_hdr hdr;

    efi_status (*raise_tpl)(u64 new_tpl);
    efi_status (*restore_tpl)(u64 old_tpl);

    efi_status (*allocate_pages)(efi_allocate_type alloc_type, efi_memory_type mem_type, size_t pages, void** memory);
    efi_status (*free_pages)(void* memory, size_t pages);

    efi_status (*get_memory_map)(size_t* memory_map_size, void* not_yet_implemented);

    u64 padding[4];
    efi_status (*wait_for_event)(u64 event_count, efi_event* events, u64* index);
    u64 padding2[6];

    efi_status (*handle_protocol)(efi_handle handle, efi_guid* protocol, void** interface);
    void* reserved;

    u64 padding3[18];

    efi_status (*locate_handle_buffer)(efi_locate_search_type search_type, efi_guid* protocol, void* search_key, size_t* handle_count, efi_handle** buffer);

} efi_boot_services;
