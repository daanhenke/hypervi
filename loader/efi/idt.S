%include "freestanding/macros.S"

bits 64
section .text

%macro idt_entry_dummy 1
__int_%1:
    push 0
    push %1
    jmp idt_common_handler
%endmacro

%macro idt_entry 1
__int_%1:
    nop
    nop
    push %1
    jmp idt_common_handler
%endmacro

%macro idt_name 1
    dq __int_%1
%endmacro

extern efi_idt_common
idt_common_handler:
    cld
    push_gp_regs

    mov rcx, rsp
    call efi_idt_common
    mov rsp, rax

    pop_gp_regs

    add rsp, 10h
    iretq

global idt_start
idt_start:
    idt_entry_dummy 0
    idt_entry_dummy 1
    idt_entry_dummy 2
    idt_entry_dummy 3
    idt_entry_dummy 4
    idt_entry_dummy 5
    idt_entry_dummy 6
    idt_entry_dummy 7

    idt_entry 8

    idt_entry_dummy 9

    idt_entry 10
    idt_entry 11
    idt_entry 12
    idt_entry 13
    idt_entry 14

    idt_entry_dummy 15
    idt_entry_dummy 16

    idt_entry 17

    idt_entry_dummy 18
    idt_entry_dummy 19
    idt_entry_dummy 20
    idt_entry_dummy 21
    idt_entry_dummy 22
    idt_entry_dummy 23
    idt_entry_dummy 24
    idt_entry_dummy 25
    idt_entry_dummy 26
    idt_entry_dummy 27
    idt_entry_dummy 28
    idt_entry_dummy 29
    idt_entry 30
    idt_entry_dummy 31

%assign i 32
%rep 224
    idt_entry_dummy i
%assign i i+1
%endrep

section .data
global interrupt_vector
interrupt_vector:
%assign i 0
%rep 256
    idt_name i
%assign i i+1
%endrep
