#include "freestanding/types.h"
#include "loader/exports.h"

extern "C" size_t visor_main()
{
    ldr_log("yeet\n");
    return reinterpret_cast<size_t>(ldr_log);
    //return 0x1337;
}
