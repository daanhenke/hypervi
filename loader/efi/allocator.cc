#include "loader/efi/allocator.h"
#include "freestanding/libc.h"
#include "loader/efi/common.h"

#define ALLOCATIONS_MAX 4096
#define ALLOCATIONS_SIZE (sizeof(efi_allocation_entry) * ALLOCATIONS_MAX)
efi_allocation_entry* efi_allocations;

void efi_allocator_init()
{
    auto status = gST->boot_services->allocate_pages(efi_allocate_type::allocate_any_pages, efi_memory_type::boot_services_data, NUM_PAGES(ALLOCATIONS_SIZE), reinterpret_cast<void**>(&efi_allocations));

    if (efi_allocations == nullptr || status != efi_status::success)
    {
        log("oh no, allocator fucked up :(\n");
        return;
    }

    _stosb(efi_allocations, 0, ALLOCATIONS_MAX);
}

void* efi_allocate(size_t size)
{
    size_t pages = NUM_PAGES(size);

    for (int i = 0; i < ALLOCATIONS_MAX; i++)
    {
        efi_allocation_entry* entry = &efi_allocations[i];

        if (entry->address != nullptr) continue;

        gST->boot_services->allocate_pages(efi_allocate_type::allocate_any_pages, efi_memory_type::boot_services_data, pages, &(entry->address));
        entry->pages = pages;
        return entry->address;
    }

    log("max allocations reached!\nshit will fuck up now\n");
    return nullptr;
}

void efi_deallocate(void* address)
{
    for (int i = 0; i < ALLOCATIONS_MAX; i++)
    {
        efi_allocation_entry* entry = &efi_allocations[i];
        if (entry->address == address)
        {
            gST->boot_services->free_pages(entry->address, entry->pages);
            entry->address = nullptr;
            entry->pages = 0;
        }
    }
}

void* operator new(size_t size)
{
    return efi_allocate(size);
}

void* operator new[](size_t size)
{
    return efi_allocate(size);
}

void operator delete(void* instance)
{
    efi_deallocate(instance);
}


void operator delete[](void* instance)
{
    efi_deallocate(instance);
}
