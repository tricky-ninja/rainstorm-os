BITS 32

  
extern isr_handler        

section .text

%macro ISR_NO_ERROR 1

global _isr%1
_isr%1:
    push 0          ; dummy error code
    push %1         ; interrupt number
    jmp _isr_common


%endmacro

%macro ISR_ERROR 1

global _isr%1
_isr%1:
    push %1         ; interrupt number (real error code already on stack)
    jmp _isr_common

%endmacro

%include "src/i686/isr.inc"

_isr_common:
    pusha

    xor eax, eax
    mov ax, ds
    push eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call isr_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp, 8
    iret   