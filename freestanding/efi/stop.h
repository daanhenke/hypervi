#pragma once

#include "freestanding/efi/core.h"
#include "freestanding/efi/status.h"

typedef struct
{
    u32 max_mode;
    u32 mode;
    u32 attribute;
    u32 cursor_col;
    u32 cursor_row;
    bool cursor_visible;
} efi_stop_mode;

struct efi_stop;
typedef struct efi_stop
{
    efi_status (*reset)(efi_stop* thiz, bool extended_verification);

    efi_status (*output_string)(efi_stop* thiz, efi_char16* string);
    efi_status (*test_string)(efi_stop* thiz, efi_char16* string);

    efi_status (*query_mode)(efi_stop* thiz, size_t number, size_t* cols, size_t* rows);
    efi_status (*set_mode)(efi_stop* thiz, size_t number);
    efi_status (*set_attribute)(efi_stop* thiz, size_t attribute);

    efi_status (*clear_screen)(efi_stop* thiz);

    efi_status (*set_cursor_position)(efi_stop* thiz, size_t col, size_t row);
    efi_status (*set_cursor_enabled)(efi_stop* thiz, bool enabled);

    efi_stop_mode* mode;
} efi_stop;