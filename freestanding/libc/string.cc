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
