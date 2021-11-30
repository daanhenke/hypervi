%include "freestanding/macros.S"

bits 64
section .text

%macro idt_entry_start 1
extern %1
global __idt_stub_%1
__idt_stub_%1:
%endmacro

%macro idt_entry_end 1
    cld
    push_gp_regs

    mov rcx, rsp
    call %1
    mov rsp, rax

    pop_gp_regs

    iretq
%endmacro

%macro idt_entry_with_error 1
    idt_entry_start %1
    idt_entry_end %1
%endmacro

%macro idt_entry_without_error 1
    idt_entry_start %1
    push 0
    idt_entry_end %1
%endmacro

idt_entry_with_error efi_idt_pagefault
idt_entry_without_error efi_idt_invalid_opcode
