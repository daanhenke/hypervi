#pragma once

#include "freestanding/efi/sfsp.h"

void efi_fs_init();
efi_fp* efi_fs_find_file(efi_char16* path, u64 open_mode, u64 attributes);
