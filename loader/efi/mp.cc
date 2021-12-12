#include "loader/efi/mp.h"
#include "loader/efi/common.h"
#include "loader/exports.h"

static efi_mp* mp = nullptr;
static efi_guid mp_guid = efi_mp_guid;

void efi_mp_init()
{
    gST->boot_services->locate_protocol(&mp_guid, nullptr, reinterpret_cast<void**>(&mp));
}

size_t efi_mp_whoami()
{
    size_t result;
    mp->whoami(mp, &result);
    return result;
}

size_t efi_mp_get_core_count()
{
    size_t result, unused;
    mp->get_processor_count(mp, &result, &unused);
    return result;
}

void efi_mp_proc(void* data)
{
    auto real_proc = reinterpret_cast<ldr_coac_cb>(data);
    real_proc();
}

void efi_mp_call_on_all_cores(ldr_coac_cb proc)
{
    proc();

    auto core_count = efi_mp_get_core_count();
    if (core_count == 1) return;

    return;

    mp->startup_all_aps(mp, efi_mp_proc, true, nullptr, 0, reinterpret_cast<void*>(proc), nullptr);
}
