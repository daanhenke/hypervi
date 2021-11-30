#pragma once

#include "freestanding/efi/core.h"
#include "freestanding/efi/status.h"

typedef void (* efi_ap_procedure)(void* data);

struct efi_mp;
typedef struct efi_mp
{
    efi_status (*get_processor_count)(efi_mp* thiz, size_t* total, size_t* enabled);
    efi_status (*dont_care)(efi_mp* thiz);
    efi_status (*startup_all_aps)(efi_mp* thiz, efi_ap_procedure procedure, bool single_thread, efi_event wait_event, u64 timeout_in_microseconds, void* data, size_t** failed_cpus);
    efi_status (*dont_care2)(efi_mp* thiz);
    efi_status (*dont_care3)(efi_mp* thiz);
    efi_status (*dont_care4)(efi_mp* thiz);
    efi_status (*whoami)(efi_mp* thiz, size_t* index);
} efi_mp;

#define efi_mp_guid {0x3fdda605, 0xa76e, 0x4f46, {0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, 0x3d, 0x08}}
