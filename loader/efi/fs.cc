#include "loader/efi/fs.h"
#include "loader/efi/common.h"

const efi_guid sfsp_guid = efi_sfsp_guid;
const efi_guid fs_label_guid = efi_filesystem_label_guid;

efi_handle** fs_handles;
efi_fp* fs_roots[64];
size_t fs_root_count = 0;

void efi_fs_init()
{
    if (gST->boot_services->locate_handle_buffer(efi_locate_search_type::by_protocol, const_cast<efi_guid*>(&sfsp_guid), nullptr, &fs_root_count, reinterpret_cast<efi_handle**>(&fs_handles)) != efi_status::success)
    {
        return;
    }

    log("Listing filesystems...\r\n");
    for (size_t i = 0; i < fs_root_count && i < 0x10; i++)
    {
        efi_sfsp* sfsp;

        if (gST->boot_services->handle_protocol(fs_handles[i], const_cast<efi_guid*>(&sfsp_guid), reinterpret_cast<void**>(&sfsp)) != efi_status::success)
        {
            continue;
        }

        sfsp->open_volume(sfsp, &fs_roots[i]);

        efi_fp_volume_label label;
        u64 buffer_size = sizeof(label);
        fs_roots[i]->get_info(fs_roots[i], const_cast<efi_guid*>(&fs_label_guid), &buffer_size, &label);

        gST->con_out->output_string(gST->con_out, label.volume_label);
        log("\r\n");
    }
}
