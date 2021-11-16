#pragma once

#include "loader/efi/common.h"

typedef struct
{
    void* address;
    size_t pages;
} efi_allocation_entry;

void efi_allocator_init();

void* efi_allocate(size_t size);
void efi_deallocate(void* address);

void* operator new(size_t size);
void* operator new[](size_t size);
void operator delete(void* instance);
