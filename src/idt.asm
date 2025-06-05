BITS 32

global _idt_load     
extern idtPtr        

section .text
_idt_load:
    lidt [idtPtr]
    ret             