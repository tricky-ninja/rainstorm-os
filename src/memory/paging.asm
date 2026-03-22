BITS 32

global _paging_switch_dir
global _paging_flush_tlb
global _paging_enable
global _paging_disable_pse
global _paging_is_enabled

section .text
_paging_switch_dir:
    mov eax, [esp+4]
    mov cr3, eax
    ret

_paging_flush_tlb:
    mov eax, cr3
    mov cr3, eax
    ret

_paging_is_enabled:
    mov eax, cr0
    shr eax, 31
    and eax, 1
    ret

_paging_enable:
    mov eax, cr0
    or eax, 0x80010000
    mov cr0, eax
    ret

_paging_disable_pse:
    mov eax, cr4
    and eax, 0xFFFFFFEF ; clear bit 4 (pse - switch back to 4kb pages)
    mov cr4, eax
    ret