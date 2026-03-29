#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct 
{
	uint16_t    addr_low;
	uint16_t    kernel_cs;
	uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
	uint8_t     flags;
	uint16_t    addr_mid; 
	uint32_t    addr_high;
	uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

typedef struct idt_descriptor
{
   uint16_t size;
   idt_entry_t *address; 
} __attribute__((packed)) idt_descriptor_t;

typedef enum 
{
    IDT_FLAG_INTERRUPT_GATE_64 = 0x0E,
    IDT_FLAG_TRAP_GATE64      = 0x0F,

    IDT_FLAG_RING_0         = (0 << 5),
    IDT_FLAG_RING_1         = (1 << 5),
    IDT_FLAG_RING_2         = (2 << 5),
    IDT_FLAG_RING_3         = (3 << 5),

    IDT_FLAG_PRESENT        = 0x80,
} IDT_FLAGS;

void idt_init();
void idt_set_gate(uint8_t num, void *addr, uint8_t flags);

