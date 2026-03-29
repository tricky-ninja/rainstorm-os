BITS 64

section .text

global _idt_load
_idt_load:
   lidt [rdi]
   ret 