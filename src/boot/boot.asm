BITS 32

MULTIBOOT_MAGIC equ 0x1badb002
MULTIBOOT_FLAG_ALIGN equ 1
MULTIBOOT_FLAG_MEMORY equ 1 << 1
MULTIBOOT_FLAG_VIDEO equ 0 << 2
MULTIBOOT_FLAG equ MULTIBOOT_FLAG_ALIGN | MULTIBOOT_FLAG_MEMORY | MULTIBOOT_FLAG_VIDEO
MULTIBOOT_CHECKSUM equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAG)

KERNEL_VMA equ 0xC0000000
PAGE_PRESENT equ 0x1
PAGE_WRITE equ 0x2
PAGE_SIZE_4MB equ 0x80

section .multiboot
    align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAG
    dd MULTIBOOT_CHECKSUM

global _start
extern kstart
extern _bss_start
extern _bss_end

section .text


_start:
    cli
    xor ebp, ebp
    mov esi, eax    ; multiboot magic
    mov edx, ebx    ; multiboot info

    ; enable 4mb paging (PSE bit in cr4)
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    ; load page directory to cr3 (we do the subtraction since paging isn't on yet and all address are in higher half)
    mov eax, (boot_page_directory - KERNEL_VMA)
    mov cr3, eax

    ; enable paging and write protect in cr0
    mov eax, cr0
    or eax, 0x80010000
    mov cr0, eax

    mov eax, higher_half
    jmp eax

higher_half:

    ; unmap identity maping and reload tlb
    mov dword [boot_page_directory], 0
    mov eax, cr3
    mov cr3, eax

    mov esp, stack_start
    lea edi, [_bss_start]
    mov ecx, _bss_end
    sub ecx, edi
    xor eax, eax        ; AL = 0
    cld
    rep stosb           ; write AL to [EDI], ECX times, incrementing EDI

bss_cleared:
    push edx    ; multiboot info
    push esi    ; multiboot magic
    call kstart
    hlt

halt:
    cli
    hlt
    jmp halt



section .data
align 4096
boot_page_directory:
    ; entry 0: identity map first 4 MB
    dd (0x00000000 | PAGE_PRESENT | PAGE_WRITE | PAGE_SIZE_4MB)
    times (768 - 1) dd 0    ; entries 1-767: not present

    ; entry 768: higher half mapping  0xC0000000 -> 0x00000000 (4MB)
    dd (0x00000000 | PAGE_PRESENT | PAGE_WRITE | PAGE_SIZE_4MB)
    times (1024 - 769) dd 0 ; not present

section .bss

; TODO: move the stack placement to linker, also add a gaurd page to protect against stack overflows
stack_end:
resb 4096 * 8   ; 32 KB
stack_start: