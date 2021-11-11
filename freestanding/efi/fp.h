#pragma once

#include "freestanding/efi/core.h"
#include "freestanding/efi/status.h"

struct efi_fp;
typedef struct efi_fp
{
    u64 revision;

    efi_status (*open)(efi_fp* thiz, efi_fp** new_handle, efi_char16* file_name, u64 open_mode, u64 attributes);
    efi_status (*close)(efi_fp* thiz);
} efi_fp;
