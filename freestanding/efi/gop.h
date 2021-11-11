#pragma once

#include "freestanding/efi/core.h"
#include "freestanding/efi/status.h"

enum class efi_gop_pixel_format : u32
{
    rgbr_8bpp,
    bgrr_8bpp,
    bitmask,
    blt_only
};

enum class efi_gop_blt_operation : u32
{
    video_fill,
    video_to_blt_buffer,
    buffer_to_video,
    video_to_video
};

typedef struct
{
    u32 red;
    u32 green;
    u32 blue;
    u32 reserved;
} efi_gop_pixel_bitmask;

typedef struct
{
    u8 blue;
    u8 green;
    u8 red;
    u8 reserved;
} efi_gop_pixel_blt;

typedef struct
{
    u32 version;
    u32 horizontal_resolution;
    u32 vertical_resolution;
    efi_gop_pixel_format pixel_format;
    efi_gop_pixel_bitmask pixel_info;
    u32 pixels_per_scanline;
} efi_gop_mode_info;

typedef struct
{
    u32 max_mode;
    u32 mode;
    efi_gop_mode_info* info;
    size_t size_of_info;
    void* framebuffer_base;
    size_t framebuffer_size;
} efi_gop_mode;

struct efi_gop;
typedef struct efi_gop
{
    efi_status (*query_mode)(efi_gop* thiz, u32 mode_number, size_t* size_of_info, efi_gop_mode_info** info);
    efi_status (*set_mode)(efi_gop* thiz, u32 mode_number);
    efi_status (*blt)(efi_gop* thiz, efi_gop_pixel_blt* blt_buffer, efi_gop_blt_operation op, size_t source_x, size_t source_y, size_t dest_x, size_t dest_y, size_t width, size_t height, size_t delta);

    efi_gop_mode* mode;
} efi_gop;

#define efi_gop_guid {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}}
