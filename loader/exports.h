#pragma once

#include "freestanding/types.h"

typedef struct
{
    char loader_name[32];
    u64 loader_version;
} hv_init_struct;

extern "C" void ldr_log(const char* string);
