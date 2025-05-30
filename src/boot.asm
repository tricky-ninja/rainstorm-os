BITS 32

MULTIBOOT_MAGIC equ 0x1badb002
MULTIBOOT_FLAG_ALIGN equ 0
MULTIBOOT_FLAG_MEMORY equ 0 << 1
MULTIBOOT_FLAG_VIDEO equ 0 << 2
MULTIBOOT_FLAG equ MULTIBOOT_FLAG_ALIGN | MULTIBOOT_FLAG_MEMORY | MULTIBOOT_FLAG_VIDEO
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAG)

section .multiboot
    align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAG
    dd MULTIBOOT_CHECKSUM

global _start
extern kstart

section .text
_start:
    cli
    mov esp, stack_start
    mov ebp, esp
    call kstart
    hlt

halt:
    cli
    hlt
    jmp halt


section .bss

stack_end:
resb 4096 * 2
stack_start: