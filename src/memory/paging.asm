BITS 64

global _paging_switch_dir
global _paging_flush_tlb
global _paging_invalidate_page
global _paging_get_current_dir
global _paging_enable_nxe

_paging_enable_nxe:
    mov ecx, 0xC0000080      ; IA32_EFER MSR
    rdmsr                    ; eax=msr
    or eax, (1 << 11)
    wrmsr
    ret

_paging_switch_dir:
    mov cr3, rdi
    ret

_paging_flush_tlb:
    mov rax, cr3
    mov cr3, rax
    ret

_paging_invalidate_page:
    invlpg [rdi]
    ret

_paging_get_current_dir:
    mov rax, cr3
    ret