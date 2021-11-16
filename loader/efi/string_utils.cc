#include "loader/efi/common.h"

void efi_string_to_cstring(efi_char16* source, char* dest)
{
    while (*source != L'\0')
    {
        *dest++ = static_cast<char>(*source++);
    }

    *dest = '\0';
}
