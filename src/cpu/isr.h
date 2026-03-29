#pragma once
#include <stdint.h>

typedef struct Registers
{
    // pushed by us
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

    // pushed by us
    uint64_t interrupt_num, error_code;

    // pushed by CPU
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) Registers;


typedef void (*isr_handler_fn)(Registers *regs);

void isr_init();
void isr_register_handler(uint8_t isr, isr_handler_fn handler);