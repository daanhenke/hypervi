bits 64
section .text

global read_cr3
read_cr3:
    mov rax, cr3
    ret

global write_cr3
write_cr3:
    mov cr3, rdi
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
