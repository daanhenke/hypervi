#include "loader/efi/idt.h"
#include "freestanding/x86utils.h"
#include "freestanding/libc.h"

typedef struct do_packed
{
    u32 Reserved0;
    u64 Rsp0;
    u64 Rsp1;
    u64 Rsp2;
    u64 Reserved1;
    u64 Ist[7];
    u64 Reserved3;
    u16 Reserved4;
    u16 IoMapBaseAddress;
} tss_entry;

typedef struct do_packed
{
    uint16_t len;
    uint16_t base_low16;
    uint8_t base_mid8;
    uint8_t flags1;
    uint8_t flags2;
    uint8_t base_high8;
    uint32_t base_upper32;
    uint32_t reserved;
} tss_gdt_entry;


static segment_descriptor_register_64 efi_gdtr = {};
static segment_descriptor_64* new_gdt = nullptr;
static tss_entry new_tss = {};

void log_gdt()
{
    for (size_t i = 0; i < efi_gdtr.limit / sizeof(segment_descriptor_64); i++)
    {
        log("found gdt entry\n");
        log_hex("type: ", new_gdt[i].type);
    }
}

void efi_gdt_init()
{
    log("patching efi gdt\n");

    segment_descriptor_register_64 new_gdtr;
    read_gdtr(&efi_gdtr);
    read_gdtr(&new_gdtr);

    size_t entries = new_gdtr.limit / sizeof(segment_descriptor_64);
    log_hex("gdt size: ", entries);

    new_gdt = new segment_descriptor_64[entries + 1];
    memcpy(new_gdt, reinterpret_cast<const void*>(new_gdtr.base_address), new_gdtr.limit);

    segment_selector tr;
    tr.flags = 0;
    tr.index = (new_gdtr.limit + 1) / sizeof(segment_descriptor_32);
    log_hex("tr index: ", tr.index);

    new_gdtr.base_address = reinterpret_cast<u64>(new_gdt);
    new_gdtr.limit += sizeof(segment_descriptor_64);

    auto new_gdt32 = reinterpret_cast<segment_descriptor_32*>(new_gdt);
    auto tss_descriptor = reinterpret_cast<tss_gdt_entry*>(&new_gdt32[tr.index]);
    auto tss_ptr = reinterpret_cast<u64>(&new_tss);

    memset(tss_descriptor, 0, sizeof(segment_descriptor_64));
    tss_descriptor->len = sizeof(new_tss) - 1;
    tss_descriptor->base_low16 = static_cast<u16>(tss_ptr);
    tss_descriptor->base_mid8 = static_cast<u8>(tss_ptr >> 16);
    tss_descriptor->base_high8 = static_cast<u8>(tss_ptr >> 24);
    tss_descriptor->base_upper32 = static_cast<u32>(tss_ptr >> 32);
    tss_descriptor->flags1 = 0b10001001;
    tss_descriptor->reserved = 0;

    memset(&new_tss, 0, sizeof(new_tss));

    write_gdtr(&new_gdtr);
    log("far jumpin\n");
    set_cs(0x38);
    log("far jumpin done\n");
    write_tr(tr.flags);
}

void efi_gdt_exit()
{
    log("resetting gdt\n");
    clear_tr();
    write_gdtr(&efi_gdtr);
    set_cs(0x38);
}
