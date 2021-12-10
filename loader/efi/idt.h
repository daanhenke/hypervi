#pragma once

#include "loader/efi/common.h"

typedef struct do_packed
{
    u64 r15;
    u64 r14;
    u64 r13;
    u64 r12;
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
    u64 rdi;
    u64 rsi;
    u64 rbp;
    u64 rbx;
    u64 rdx;
    u64 rcx;
    u64 rax;

    u64 error_code;
    u64 rip;
    u64 cs;
    u64 rflags;
    u64 rsp;
    u64 ss;
} idt_ctx;

void efi_idt_init();
void efi_idt_exit();

#define def_stub(name) extern "C" void __idt_stub_ ## name ()
#define def_handler(name) extern "C" idt_ctx* name (idt_ctx* ctx)
#define def_hook(name) def_stub(name); def_handler(name)

def_hook(efi_idt_pagefault);
def_hook(efi_idt_invalid_opcode);
def_hook(efi_idt_general_protection_fault);
def_hook(efi_idt_stack_segment_fault);
def_hook(efi_idt_invalid_tss);
def_hook(efi_idt_segment_not_present);
