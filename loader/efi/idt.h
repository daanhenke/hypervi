#pragma once

void efi_idt_init();
void efi_idt_exit();

extern "C" u64 efi_idt_common(u64 rsp);
extern "C" void idt_start();
