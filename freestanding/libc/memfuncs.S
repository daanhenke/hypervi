bits 64
section .text

global _stosb
_stosb:
    cld
    mov rcx, rdx
    rep stosb
    ret

global _movsb
_movsb:
    cld
    mov rcx, rdx
    rep movsb
    ret
