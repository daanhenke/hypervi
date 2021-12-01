#pragma once

#include "freestanding/efi.h"
#include "loader/efi/console.h"

extern efi_system_table* gST;

#define log(str) efi_console_write(str)
#define log_hex(str, thing) log(str); efi_console_hex(thing); log("\n")

void efi_string_to_cstring(efi_char16* source, char* dest);

