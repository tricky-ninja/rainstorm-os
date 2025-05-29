BITS 64

MULTIBOOT_MAGIC equ 0x1badb002
MULTIBOOT_FLAG_ALIGN equ 0
MULTIBOOT_FLAG_MEMORY equ 0 << 1
MULTIBOOT_FLAG_VIDEO equ 0 << 2
MULTIBOOT_FLAG equ MULTIBOOT_FLAG_ALIGN | MULTIBOOT_FLAG_MEMORY | MULTIBOOT_FLAG_VIDEO
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAG)

section .multiboot
    align 8
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAG
    dd MULTIBOOT_CHECKSUM

global _start
extern kstart

section .text
_start:
    cli
    mov rsp, stack_start
    mov rbp, rsp
    call kstart
    hlt

halt:
    cli
    hlt
    jmp halt


section .bss

stack_end:
resb 16384
stack_start: