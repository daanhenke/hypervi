#include "freestanding/efi.h"
#include "loader/efi/common.h"
#include "loader/efi/console.h"

efi_system_table* gST;

efi_status efi_main(efi_handle module_handle, efi_system_table* st)
{
    gST = st;

    efi_console_init();
    st->con_out->output_string(st->con_out, ESTR("Custom EFI lib\r\n"));

    return efi_status::success;
}
