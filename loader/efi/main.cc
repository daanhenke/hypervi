#include "freestanding/efi.h"
#include "loader/efi/common.h"
#include "loader/efi/gfx.h"
#include "loader/efi/fs.h"
#include "loader/efi/input.h"

efi_system_table* gST;

efi_status efi_main(efi_handle module_handle, efi_system_table* st)
{
    gST = st;

    efi_gfx_init();
    efi_fs_init();

    efi_input_wait_for_key();

    return efi_status::success;
}
