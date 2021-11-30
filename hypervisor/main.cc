#include "freestanding/types.h"
#include "loader/exports.h"

void visor_init_core()
{
    ldr_log("hello from a core!\n");
}

size_t visor_init()
{
    ldr_log("checking hardware support...\n");
    ldr_log("setting up cores...\n");
    ldr_call_on_all_cores(visor_init_core);
    return 0;
}

extern "C" size_t visor_main(hv_init_struct* init_struct)
{
    ldr_log("hello from hypervisor\n");

    size_t status = 0;

    status = visor_init();

    return status;
}
