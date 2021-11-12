#pragma once

#include "freestanding/efi/core.h"
#include "freestanding/efi/stop.h"
#include "freestanding/efi/boot_services.h"

typedef struct
{
    efi_table_hdr hdr;

    efi_char16* firmware_vendor;
    u32 firwmare_revision;

    efi_handle handle_con_in;
    efi_handle con_in;

    efi_handle handle_con_out;
    efi_stop* con_out;

    efi_handle handle_con_err;
    efi_stop* con_err;

    efi_boot_services* runtime_services;
    efi_boot_services* boot_services;
} efi_system_table;