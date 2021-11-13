#include "loader/efi/input.h"
#include "loader/efi/common.h"

void efi_input_wait_for_key()
{
    gST->boot_services->wait_for_event(1, &gST->con_in->wait_for_key, nullptr);
}
