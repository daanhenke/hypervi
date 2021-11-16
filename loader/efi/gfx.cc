#include "loader/efi/gfx.h"
#include "loader/efi/common.h"
#include "bitmapfont.png.h"

const efi_guid gop_guid = efi_gop_guid;
efi_gop* gop = nullptr;

void efi_gfx_blit(const unsigned char* image_data, size_t image_width, size_t width, size_t height, size_t dest_x, size_t dest_y, size_t source_x, size_t source_y)
{
    gop->blt(gop, reinterpret_cast<efi_gop_pixel_blt*>(const_cast<unsigned char*>(image_data)), efi_gop_blt_operation::buffer_to_video, source_x, source_y, dest_x, dest_y, width, height, image_width * 4);
}

void efi_gfx_char(size_t x, size_t y, char character)
{
    if (character == '\0') return;

    size_t char_width = bitmapfont_image_width / 32;
    size_t char_height = bitmapfont_image_height / 8;
    size_t char_x = character % 32;
    size_t char_y = character / 32;
    efi_gfx_blit(bitmapfont_image_data, bitmapfont_image_width, char_width, char_height, x, y, char_x * char_width, char_y * char_height);
}

void efi_gfx_string(size_t x, size_t y, char* string)
{
    if (string == nullptr) return efi_gfx_string(x, y, "<nullptr>");
    size_t start_x = x;
    while (true)
    {
        char current = *string++;

        switch (current)
        {
        case '\0':
            return;

        case '\n':
            x = start_x;
            y += bitmapfont_image_height / 8;
            break;

        default:
            efi_gfx_char(x, y, current);
            x += bitmapfont_image_width / 32;
            break;
        }
    }
}

void efi_gfx_string(size_t x, size_t y, const char* string)
{
    efi_gfx_string(x, y, const_cast<char*>(string));
}

void efi_gfx_init()
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

void efi_gfx_get_res(size_t* width, size_t* height)
{
    *width = gop->mode->info->horizontal_resolution;
    *height = gop->mode->info->vertical_resolution;
}

void efi_gfx_get_char_size(size_t* width, size_t* height)
{
    *width = bitmapfont_image_width / 32;
    *height = bitmapfont_image_height / 8;
}
