#include "freestanding/libc.h"

int itoa(unsigned long long value, char* buff, unsigned int radix, int sign, unsigned int pad)
{
    int neg = 0;
    char *buff_ptr = buff;
    unsigned int i, len;

    if (radix > 16)
    {
        *buff = '\0';
        return 0;
    }

    if (sign && value < 0)
    {
        neg = 1;
        value = -value;
    }

    do
    {
        int digit = value % radix;
        *(buff_ptr++) = (digit < 10 ? '0' + digit : 'A' + digit - 10);
        value /= radix;
    }
    while (value > 0);

    for (i = buff_ptr - buff; i < pad; i++)
    {
        *(buff_ptr++) = 0;
    }

    if (neg)
    {
        *(buff_ptr++) = '-';
    }

    *buff_ptr = '\0';

    len = buff_ptr - buff;

    for (i = 0; i < len / 2; i ++)
    {
        char tmp = buff[i];
        buff[i] = buff[len - i - 1];
        buff[len - i - 1] = tmp;
    }

    return len;
}
