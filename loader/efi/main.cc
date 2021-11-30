#include "freestanding/efi.h"
#include "loader/efi/common.h"
#include "loader/efi/gfx.h"
#include "loader/efi/loader.h"
#include "loader/efi/input.h"
#include "loader/efi/console.h"
#include "loader/efi/allocator.h"
#include "loader/efi/paging.h"
#include "loader/efi/gdt.h"
#include "loader/efi/idt.h"

efi_system_table* gST;

efi_status efi_main(efi_handle module_handle, efi_system_table* st)
{
    gST = st;

    efi_gfx_init();
    efi_console_init();

    efi_allocator_init();
    efi_fs_init();

    //efi_paging_init();
    //efi_gdt_init();
    efi_idt_init();

    efi_loader_init(); // deze 2 weg is geen crash
    //efi_loader_map();

    efi_idt_exit();

    log("loader executed succesfully, press any key to continue...\n");

    efi_input_wait_for_key();

    return efi_status::success;
}
