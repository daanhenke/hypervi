#pragma once

#include "freestanding/efi/core.h"
#include "freestanding/efi/status.h"
#include "freestanding/efi/fp.h"

struct efi_sfsp;
typedef struct efi_sfsp
{
    u64 revision;
    efi_status (*open_volume)(efi_sfsp* thiz, efi_fp** root_fp);
} efi_sfsp;

#define efi_sfsp_guid {0x0964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}
