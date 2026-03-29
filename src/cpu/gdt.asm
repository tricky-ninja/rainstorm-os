BITS 64

global _gdt_flush

section .text

_gdt_flush:
    lgdt [rdi]  ; rdi = gdtptr

    ; reload the code segment
    push 0x8
    lea rax, [.reload_segments]
    push rax
    retfq   ; jumps to 0x8:.reload_segments

    .reload_segments:
        mov ax, 0x10    ; kernel data segment
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        ret