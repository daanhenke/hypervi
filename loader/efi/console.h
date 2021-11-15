#pragma once

#include "loader/efi/common.h"

typedef struct
{
    size_t rows;
    size_t columns;

    size_t current_row;
    size_t current_column;

    char* buffer;
    size_t buffer_size;

    size_t char_width;
    size_t char_height;
} efi_console_ctx;

void efi_console_init();
void efi_console_fill(char character, char attribute);
void efi_console_draw();
void efi_console_write(char* message);
