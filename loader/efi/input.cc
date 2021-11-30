#include "loader/efi/input.h"
#include "loader/efi/common.h"

void efi_input_wait_for_key()
{
    efi_stip_key key;
    while (true)
    {
        gST->boot_services->wait_for_event(1, &gST->con_in->wait_for_key, nullptr);

        gST->con_in->read_key_stroke(gST->con_in, &key);
        gST->con_in->reset(gST->con_in, true);

        if (key.unicode_char != 0 || key.scan_code != 0) break;
    }
}
