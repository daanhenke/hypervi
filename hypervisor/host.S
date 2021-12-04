%include "freestanding/macros.S"
bits 64

section .data
global host_vmexit_ptr
host_vmexit_ptr: dq

section .text

global host_hv_entrypoint
host_hv_entrypoint:
    push_gp_regs

    ; sub rsp, 68h
    ; movaps [rsp + 0], xmm0
    ; movaps [rsp + 10h], xmm1
    ; movaps [rsp + 20h], xmm2
    ; movaps [rsp + 30h], xmm3
    ; movaps [rsp + 40h], xmm4
    ; movaps [rsp + 50h], xmm5

    mov rdi, rsp
    sub rsp, 20h
    mov rsi, host_vmexit_ptr
    call [rsi]
    add rsp, 20h

    ; movaps xmm5, [rsp + 50h]
    ; movaps xmm4, [rsp + 40h]
    ; movaps xmm3, [rsp + 30h]
    ; movaps xmm2, [rsp + 20h]
    ; movaps xmm1, [rsp + 10h]
    ; movaps xmm0, [rsp + 0]
    ; add rsp, 68h

    ; todo exit

    pop_gp_regs
    vmresume
    jmp .error
.error:
    int 3
    hlt
    jmp .error

