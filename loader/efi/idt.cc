#include "loader/efi/common.h"
#include "loader/efi/idt.h"
#include "freestanding/x86utils.h"

typedef struct do_packed {
	u16 isr_low;
	u16 kernel_cs;
	u8 ist;
	u8 attributes;
	u16 isr_mid;
	u32 isr_high;
	u32 reserved;
} idt_entry_t;

#define idt_size 256

typedef struct do_packed
{
    idt_entry_t entries[idt_size];
} idt_t;

static idt_t idt = {};

extern "C" u64 interrupt_vector[idt_size];

static segment_descriptor_register_64 original_idtr = {};

void efi_idt_init()
{
    log("initializing idt\n");

    segment_descriptor_register_64 idtr;
    original_idtr.base_address = 0;
    read_idtr(&original_idtr);

    log_hex("current idt base: ", original_idtr.base_address);

    for (size_t i = 0; i < idt_size; i++)
    {
        size_t idt_handler = interrupt_vector[i];
        idt.entries[i].isr_low = static_cast<u16>(idt_handler);
        idt.entries[i].kernel_cs = 0x38;
        idt.entries[i].ist = 0;
        idt.entries[i].reserved = 0;
        idt.entries[i].attributes = 0x8E;
        idt.entries[i].isr_mid = static_cast<u16>(idt_handler >> 16);
        idt.entries[i].isr_high = static_cast<u32>(idt_handler >> 32);
    }

    idtr.base_address = reinterpret_cast<u64>(&idt);
    idtr.limit = sizeof(idt) - 1;

    log_hex("setting idtr to ", idtr.base_address);
    write_idtr(&idtr);
    log("custom interrupt handler active!\n");
}

void efi_idt_exit()
{
    log_hex("setting back old idt @ ", original_idtr.base_address);
    write_idtr(&original_idtr);
    log("disabled idt!\n");
}

u64 efi_idt_common(u64 rsp)
{
    log_hex("idt called, rsp: ", rsp);
    return rsp;
}
