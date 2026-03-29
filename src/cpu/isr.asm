BITS 64

extern isr_handler

section .text

%macro ISR_NO_ERROR 1
global _isr%1
_isr%1:
    push 0  ; dummy error code
    push %1 ; interupt number
    jmp _isr_common
%endmacro

%macro ISR_ERROR 1
global _isr%1
_isr%1:
    push %1 ; interupt number (error code already on stack)
    jmp _isr_common
%endmacro

%include "src/cpu/isr_gen.inc"

_isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call isr_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16 ; pop interrupt num and error code from stack
    iretq
