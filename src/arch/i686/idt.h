/**
 *  Module: Interrupt Descriptor Table (IDT)
 *
 *  Registers interrupt and exception handlers.
 *
 *  Required before enabling hardware interrupts.
 *  Author: Sreyas A (TrickyNinja)
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct idt_entry
{
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t always0;
    uint8_t flags;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry ;

typedef struct idt_descriptor
{
   uint16_t size;
   idt_entry *address; 
} __attribute__((packed)) idt_descriptor;

typedef enum IDT_FLAGS
{
    IDT_FLAG_TASK_GATE = 0x5,
    IDT_FLAG_INTERUPT_GATE_32 = 0xE,
    IDT_FLAG_TRAP_GATE_32 = 0xF,

    IDT_FLAG_RING_0 = (0 << 5),
    IDT_FLAG_RING_1 = (1 << 5),
    IDT_FLAG_RING_2 = (2 << 5),
    IDT_FLAG_RING_3 = (3 << 5),

    IDT_FLAG_PRESENT = 0x80,

} IDT_FLAGS;

extern idt_entry idt[256];
extern idt_descriptor idtPtr;

void idt_set_gate(uint8_t num, uint32_t offset, uint16_t segment, uint8_t flags);
void idt_install();

void _idt_load();