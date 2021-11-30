#include "loader/efi/common.h"
#include "loader/efi/gdt.h"
#include "freestanding/x86utils.h"

void log_gdt(segment_descriptor_register_64* gdtr)
{
    auto curr_seg = reinterpret_cast<segment_descriptor_64*>(gdtr->base_address);
    auto end = reinterpret_cast<segment_descriptor_64*>(reinterpret_cast<char*>(curr_seg) + gdtr->limit);

    while (curr_seg < end)
    {
        log("found gdt segment\n");
        curr_seg++;
    }
}

void efi_gdt_init()
{
    log("initializing gdt\n");

    segment_descriptor_register_64 gdtr;
    gdtr.base_address = 0;
    read_gdtr(&gdtr);

    log_hex("current gdt base: ", gdtr.base_address);
    log_gdt(&gdtr);
}
