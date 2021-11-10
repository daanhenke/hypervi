#include "freestanding/efi.h"

efi_system_table* gST;

void efi_console_init()
{
    auto out = gST->con_out;
    
    s32 best_i = out->mode->mode;
    size_t best_cols = 0;
    size_t best_rows = 0;

    // Select the mode with the highest console resolution
    for (u32 i = 0; i < out->mode->max_mode; i++)
    {
        size_t rows, cols;
        out->query_mode(out, i, &cols, &rows);

        if (cols >= best_cols || rows >= best_rows)
        {
            best_cols = cols;
            best_rows = rows;
            best_i = i;
        }
    }

    // If we found something better than we already have change to it
    if (best_cols > 0 && best_rows > 0)
    {
        out->set_mode(out, best_i);
        out->clear_screen(out);
    }
}

efi_status efi_main(efi_handle module_handle, efi_system_table* st)
{
    gST = st;

    efi_console_init();
    st->con_out->output_string(st->con_out, L"Custom EFI lib\r\n");

    return efi_status::success;
}