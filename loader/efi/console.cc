#include "loader/efi/common.h"
#include "bitmapfont.png.h"

const efi_guid gop_guid = efi_gop_guid;
efi_gop* gop = nullptr;

#define log(str) gST->con_out->output_string(gST->con_out, ESTR(str))

void efi_console_init()
{
    auto boot = gST->boot_services;

    efi_handle* graphics_handles = nullptr;
    size_t graphics_handle_count = 0;
    if (boot->locate_handle_buffer(efi_locate_search_type::by_protocol, const_cast<efi_guid*>(&gop_guid), nullptr, &graphics_handle_count, &graphics_handles) != efi_status::success)
    {
        // TODO: PANIC
        log("Failed to get handle buffer\r\n");
        return;
    }

    for (size_t i = 0; i < graphics_handle_count; i++)
    {
        if (boot->handle_protocol(graphics_handles[i], const_cast<efi_guid*>(&gop_guid), reinterpret_cast<void**>(&gop)) == efi_status::success)
        {
            break;
        }
    }

    if (gop == nullptr)
    {
        // TODO: PANIC
        log("Failed to get gop\r\n");
    }

    auto current_mode = gop->mode->mode;
    auto current_width = gop->mode->info->horizontal_resolution;

    size_t info_size;
    efi_gop_mode_info* info;
    for (size_t i = 0; i < gop->mode->max_mode; i++)
    {
        if (gop->query_mode(gop, i, &info_size, &info) != efi_status::success)
        {
            continue;
        }

        if (info->horizontal_resolution >= current_width)
        {
            current_mode = i;
            current_width = info->horizontal_resolution;
        }
    }

    if (current_mode != gop->mode->mode)
    {
        if (gop->set_mode(gop, current_mode) != efi_status::success)
        {
            // TODO: PANIC
            return;
        }
    }

    auto col = reinterpret_cast<efi_gop_pixel_blt*>(const_cast<unsigned char*>(bitmapfont_image_data));
    gop->blt(gop, col, efi_gop_blt_operation::video_fill, 0, 0, 0, 0, gop->mode->info->horizontal_resolution, gop->mode->info->vertical_resolution, 0);
}
