#include "freestanding/libc.h"

void* memcpy(void* dest, const void* source, size_t num)
{
    _movsb(dest, const_cast<void*>(source), num);
    return dest;
}
