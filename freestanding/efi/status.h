#pragma once

#include "freestanding/types.h"

enum class efi_status : u64
{
    success = 0,
    error = 0x8000000000000000
};