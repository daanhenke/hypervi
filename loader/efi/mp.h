#pragma once

#include "loader/exports.h"

void efi_mp_init();

size_t efi_mp_whoami();
size_t efi_mp_get_core_count();
void efi_mp_call_on_all_cores(ldr_coac_cb proc);
