BITS 64

extern kmain

global _start
section .text
_start:
    xor rbp, rbp
    mov rsp, stack_top
    call kmain

section .bss
align 16
stack_bottom:
resb 65536
stack_top: