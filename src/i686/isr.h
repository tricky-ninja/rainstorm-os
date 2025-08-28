#pragma once
#include "i686/idt.h"
#include <stdint.h>

typedef struct Registers
{
    uint32_t ds;            // pushed by us
    uint32_t edi, esi, ebp, kern_esp, ebx, edx, ecx, eax;    // pushed by us
    uint32_t interrupt_num, error_code;     // Pushed by isr_handler in asm
    uint32_t eip, cs, eflags, esp, ss;      // Pushed by cpu
}__attribute__((packed)) Registers;

typedef void (*ISRHandler)(Registers* regs);

void isr_register_handler(uint8_t isr, ISRHandler handler);

void isr_install();