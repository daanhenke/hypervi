#include "loader/efi/console.h"
#include "loader/efi/gfx.h"
#include "logo.png.h"

efi_console_ctx ctx;

void efi_console_init()
{
    size_t screen_width, screen_height;
    efi_gfx_get_res(&screen_width, &screen_height);

    size_t char_width, char_height;
    efi_gfx_get_char_size(&char_width, &char_height);
    ctx.rows = (screen_width - logo_image_width) / char_width;
    ctx.columns = screen_height / char_height;

    ctx.char_width = char_width;
    ctx.char_height = char_height;

    ctx.current_row = 0;
    ctx.current_column = 0;

    ctx.buffer_size = ctx.rows * ctx.columns * sizeof(char) * 2;
    gST->boot_services->allocate_pages(efi_allocate_type::allocate_any_pages, efi_memory_type::boot_services_data, NUM_PAGES(ctx.buffer_size), reinterpret_cast<void**>(&ctx.buffer));
}

void efi_console_setcell(size_t x, size_t y, char character, char attribute)
{
    size_t idx = y * ctx.rows + x;
    ctx.buffer[idx] = character;
    ctx.buffer[idx + 1] = attribute;
}

void efi_console_fill(char character, char attribute)
{
    for (size_t y = 0; y < ctx.columns; y++)
    {
        for (size_t x = 0; x < ctx.rows; x++)
        {
            efi_console_setcell(x, y, character, attribute);
        }
    }
}

void efi_console_draw()
{
    for (size_t y = 0; y < ctx.columns; y++)
    {
        for (size_t x = 0; x < ctx.rows; x++)
        {
            size_t idx = y * ctx.rows + x;
            char character = ctx.buffer[idx];

            efi_gfx_char(x * ctx.char_width, y * ctx.char_height, character);
        }
    }

    efi_gfx_blit(logo_image_data, logo_image_width, logo_image_width, logo_image_height, 1920 - logo_image_width, 1080 - logo_image_height, 0, 0);
}

void efi_console_write(char* message)
{
    char* curr_char = message;

    while (*curr_char != '\0')
    {
        switch (*curr_char)
        {
        case '\n':
            ctx.current_row = 0;
            ctx.current_column++;
            break;

        case '\t':
            ctx.current_row += 4;
            break;

        default:
            efi_console_setcell(ctx.current_row, ctx.current_column, *curr_char, 0);
            ctx.current_row++;
            break;
        }

        curr_char++;
    }

    efi_console_draw();
}
