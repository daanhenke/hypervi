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

%macro rw_r16 1
global read_%1
read_%1:
    mov ax, %1
    ret

global write_%1
write_%1:
    mov %1, di
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
rw_cr dr7

rw_r16 es
rw_r16 cs
rw_r16 ss
rw_r16 ds
rw_r16 fs
rw_r16 gs

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

global write_msr
write_msr:
    mov ecx, edi
    mov eax, esi
    shr rsi, 32
    mov edx, esi
    wrmsr
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

global read_ldtr
read_ldtr:
    sldt ax
    ret

global call_lsl
call_lsl:
    lsl rax, rdi
    ret

global call_lar
call_lar:
    mov rax, 0
    lar rax, rdi
    jz .done
    xor rax, rax
.done:
    ret

global read_rflags
read_rflags:
    pushfq
    pop rax
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

global call_bsf
call_bsf:
    bsf rax, rdi
    ret

global get_rsp
get_rsp:
    mov rax, rsp
    add rax, 8
    ret

global get_rip
get_rip:
    mov rax, [rsp]
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

global call_vmwrite
call_vmwrite:
    vmwrite rdi, rsi
    jmp vmx_error_tail

global call_vmread
call_vmread:
    vmread [rsi], rdi
    jmp vmx_error_tail

global call_vmlaunch
call_vmlaunch:
    vmlaunch
    jmp vmx_error_tail
