#include "freestanding/libc.h"

int strcmp(const char* a, const char* b)
{
    while (*a)
    {
        if (*a != *b) {
            break;
        }

        a++;
        b++;
    }

    return *reinterpret_cast<const unsigned char*>(a) - *reinterpret_cast<const unsigned char*>(b);
}

size_t strlen(const char* str)
{
    auto orig_ptr = str;
    while(*(str++) != '\0');
    return str - orig_ptr;
}
