#include "loader/efi/idt.h"
#include "freestanding/x86utils.h"
#include "freestanding/libc.h"

typedef struct do_packed {
	u16 isr_low;
	u16 kernel_cs;
	u8 ist;
	u8 attributes;
	u16 isr_mid;
	u32 isr_high;
	u32 reserved;
} idt_entry_t;

typedef struct
{
    u8 index;
    void* handler;
} idt_hook_t;

static idt_hook_t hooks[] =
{
    { 0x6, __idt_stub_efi_idt_invalid_opcode },
    { 0xD, __idt_stub_efi_idt_general_protection_fault },
    { 0xE, __idt_stub_efi_idt_pagefault }
};
static const size_t hook_count = sizeof(hooks) / sizeof(idt_hook_t);

static segment_descriptor_register_64 efi_idtr = {};
static idt_entry_t* custom_idt;
static segment_descriptor_register_64 custom_idtr = {};

void efi_idt_init()
{
    log("initializing idt hooks\n");

    read_idtr(&efi_idtr);
    read_idtr(&custom_idtr);

    size_t entries = efi_idtr.limit / sizeof(idt_entry_t);
    log_hex("idt size: ", entries);

    custom_idt = new idt_entry_t[entries];
    memcpy(custom_idt, reinterpret_cast<const void*>(efi_idtr.base_address), efi_idtr.limit);

    log("hooking interesting entries\n");
    for (size_t i = 0; i < hook_count; i++)
    {
        auto hook = hooks[i];
        auto entry = &custom_idt[hook.index];
        auto ptr = reinterpret_cast<u64>(hook.handler);
        entry->isr_low = static_cast<u16>(ptr);
        entry->isr_mid = static_cast<u16>(ptr >> 16);
        entry->isr_high = static_cast<u32>(ptr >> 32);

        log_hex("kernel cs: ", entry->kernel_cs);
    }

    log("overwriting idtr\n");
    custom_idtr.base_address = reinterpret_cast<u64>(custom_idt);
    write_idtr(&custom_idtr);
}

void efi_idt_exit()
{
    write_idtr(&efi_idtr);
    delete[] custom_idt;
}

u64 efi_idt_pagefault(idt_ctx* ctx)
{
    auto fucked_addr = read_cr2();
    log("\n\n\noh no, page fault :((((((((((((((((((((((((((((\n");
    log_hex("naughty ptr: ", reinterpret_cast<u64>(fucked_addr));
}

u64 efi_idt_invalid_opcode(idt_ctx* ctx)
{
    log("\n\n\noh no, invalid instruction :((((((((((((((((((((((((((((\n");
    log_hex("naughty rip: ", reinterpret_cast<u64>(ctx->rip));
}


u64 efi_idt_general_protection_fault(idt_ctx* ctx)
{
    log("\n\n\noh no, general protection fault :((((((((((((((((((((((((((((\n");
    log_hex("naughty error: ", reinterpret_cast<u64>(ctx->error_code));
}
