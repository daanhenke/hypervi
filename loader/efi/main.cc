#include "freestanding/efi.h"
#include "loader/efi/common.h"
#include "loader/efi/gfx.h"
#include "loader/efi/fs.h"
#include "loader/efi/input.h"
#include "loader/efi/console.h"

efi_system_table* gST;

efi_status efi_main(efi_handle module_handle, efi_system_table* st)
{
    gST = st;

    efi_gfx_init();
    efi_console_init();
    efi_fs_init();

    efi_console_write("Hello World\n\tEng\n\tOwO");
    efi_console_write("\n\n\n\n\t\t---very cool---");

    efi_input_wait_for_key();

    return efi_status::success;
}
