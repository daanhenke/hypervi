#pragma once

#include "freestanding/types.h"

typedef struct
{
    char loader_name[32];
    u64 loader_version;
} hv_init_struct;

extern "C" void ldr_log(const char* string);
extern "C" void ldr_log_hex(size_t value);

extern "C" size_t ldr_core_whoami();
extern "C" size_t ldr_core_count();

typedef __attribute__((sysv_abi)) void (*ldr_coac_cb)();
extern "C" void ldr_call_on_all_cores(ldr_coac_cb function);

extern "C" void* ldr_malloc(size_t size);
