#include "freestanding/types.h"
#include "loader/exports.h"

extern "C" size_t visor_main(hv_init_struct* init_struct)
{
    ldr_log("yeet\n");
    ldr_log(init_struct->loader_name);
    ldr_log("\n");
    return reinterpret_cast<size_t>(ldr_log);
    //return 0x1337;
}
