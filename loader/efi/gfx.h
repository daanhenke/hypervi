#pragma once

void efi_gfx_init();

void efi_gfx_blit(const unsigned char* image_data, size_t image_width, size_t width, size_t height, size_t dest_x, size_t dest_y, size_t source_x, size_t source_y);
void efi_gfx_char(size_t x, size_t y, char character);
void efi_gfx_string(size_t x, size_t y, char* string);
void efi_gfx_string(size_t x, size_t y, const char* string);
