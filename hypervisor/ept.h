#pragma once

#include "freestanding/types.h"
#include "freestanding/x86utils.h"

typedef struct
{
    ept_pointer ptr;
    ept_pml4 pml4;
    epdpte pdpt[512];
    epde pde[512][512];
    epte pt[8][512][512];
} ept_state;

void ept_init_mtrr();
u64 ept_get_memory_type(u64 start, u64 size);

struct cpu_state;
void ept_init_core(cpu_state* state);
u64 ept_get_ptr(cpu_state* state);

void ept_invalidate_ept_cache(void* ept_ptr);
void ept_invalidate_vpid_cache(u16 vproc_id);
