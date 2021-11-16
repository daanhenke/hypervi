#include "freestanding/efi.h"
#include "loader/efi/common.h"
#include "loader/efi/gfx.h"
#include "loader/efi/loader.h"
#include "loader/efi/input.h"
#include "loader/efi/console.h"
#include "loader/efi/allocator.h"

efi_system_table* gST;

efi_status efi_main(efi_handle module_handle, efi_system_table* st)
{
    gST = st;

    efi_gfx_init();
    efi_console_init();

    efi_allocator_init();
    efi_fs_init();
    efi_loader_init();

    log("Hello World\n\tEng\n\tOwO");
    log("\n\n\n\n\t\t---very cool---\n");
    efi_console_hex(0x1337042012345678);
    log("\n");

    efi_loader_map();

    efi_input_wait_for_key();

    return efi_status::success;
}
