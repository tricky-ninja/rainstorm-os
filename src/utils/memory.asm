BITS 64

global memcpy

; void *memcpy(void *restrict dest, const void *restrict src, size_t count)
memcpy:
    push rdi
    mov rcx, rdx 
    shr rcx, 3  ; count / 8
    rep movsq   ; move 8 bytes from rsi to rdi then increment rsi and rdi by 8 and repeat this rcx times

    mov rcx, rdx
    and rcx, 7  ; count % 8
    rep movsb   ; move 1 byte from rsi to rdi then increment rsi and rdi by 1 and repeat this rcx times
    pop rax     ; return dest
    ret