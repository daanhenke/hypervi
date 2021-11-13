#pragma once

#include "freestanding/efi/core.h"
#include "freestanding/efi/status.h"

typedef struct
{
    u16 scan_code;
    u16 unicode_char;
} efi_stip_key;

struct efi_stip;
typedef struct efi_stip
{
    efi_status (*reset)(efi_stop* thiz, bool extended_verification);
    efi_status (*read_key_stroke)(efi_stop* thiz, efi_stip_key* key);

    efi_event wait_for_key;
} efi_stip;
