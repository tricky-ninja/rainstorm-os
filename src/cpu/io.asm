BITS 64

global outb
global inb
global io_wait

section .text

outb:
    mov dx, di
    mov al, sil
    out dx, al
    ret

inb:
    mov dx, di
    xor eax, eax
    in al, dx
    ret 

io_wait:
    xor eax, eax
    out 0x80, al
    ret