#pragma once

#include "freestanding/efi.h"

extern efi_system_table* gST;

#define log(str) gST->con_out->output_string(gST->con_out, ESTR(str))

// void* operator new(size_t size)
// {

// }

// void operator delete(void* instance)
// {

// }
