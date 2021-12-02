bits 64
section .text

%macro rw_cr 1
global read_%1
read_%1:
    mov rax, %1
    ret

global write_%1
write_%1:
    mov %1, rdi
    ret
%endmacro

%macro rw_ir 1
global read_%1r
read_%1r:
    s%1 [rdi]
    ret

global write_%1r
write_%1r:
    l%1 [rdi]
    ret
%endmacro

rw_cr cr0
rw_cr cr1
rw_cr cr2
rw_cr cr3
rw_cr cr4

rw_ir gdt

global read_msr
read_msr:
    push rbp
    mov rbp, rsp

    mov rcx, rdi
    rdmsr

    shl rdx, 32
    or rax, rdx

    leave
    ret

global read_idtr
read_idtr:
    sidt [rdi]
    ret

global write_idtr
write_idtr:
    cli
    lidt [rdi]
    sti
    ret

global read_tr
read_tr:
    str ax
    ret

global write_tr
write_tr:
    ltr di
    ret

global clear_tr
clear_tr:
    clts
    ret

global call_cpuid
call_cpuid:
    push rbx

    mov eax, [rdi]
    mov ecx, [rdi + 8]

    cpuid

    mov [rdi], eax
    mov [rdi + 4], ebx
    mov [rdi + 8], ecx
    mov [rdi + 12], edx

    pop rbx
    ret

global set_cs
set_cs:
    mov rax, qword .done
    push rdi
    push rax
    o64 retf
.done:
    ret

; all vmx instructions share a common method of checking if they succeeded
vmx_error_tail:
    jz .error_with_code
    jc .error_no_code
    xor rax, rax
    ret
.error_with_code:
    mov rax, 1
    ret
.error_no_code:
    mov rax, 2
    ret

global call_vmxon
call_vmxon:
    push rdi
    clc
    vmxon [rsp]
    pop rdi
    jmp vmx_error_tail

global call_invept
call_invept:
    invept rdi, [rsi]
    jmp vmx_error_tail

global call_invvpid
call_invvpid:
    invvpid rdi, [rsi]
    jmp vmx_error_tail

global call_vmclear
call_vmclear:
    push rdi
    vmclear [rsp]
    pop rdi
    jmp vmx_error_tail

global call_vmptrld
call_vmptrld:
    push rdi
    vmptrld [rsp]
    pop rdi
    jmp vmx_error_tail
