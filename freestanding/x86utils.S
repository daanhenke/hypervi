bits 64
section .text

global read_cr2
read_cr2:
    mov rax, cr2
    ret

global write_cr2
write_cr2:
    mov cr2, rdi
    ret


global read_cr3
read_cr3:
    mov rax, cr3
    ret

global write_cr3
write_cr3:
    mov cr3, rdi
    ret


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


global read_gdtr
read_gdtr:
    sgdt [rdi]
    ret

global write_gdtr
write_gdtr:
    lgdt [rdi]
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
