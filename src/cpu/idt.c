#include "idt.h"
#include "isr.h"
#include "utils/memory.h"

static idt_entry_t idt[256];
idt_descriptor_t idtPtr;

void _idt_load(idt_descriptor_t *idtptr);

void idt_set_gate(uint8_t num, void *addr, uint8_t flags)
{
    idt[num].addr_low = (uint16_t)((uint64_t)addr & 0xFFFF);
    idt[num].addr_mid = (uint16_t)(((uint64_t)addr >> 16) & 0xFFFF);
    idt[num].addr_high = (uint32_t)(((uint64_t)addr >> 32) & 0xFFFFFFFF);
    idt[num].kernel_cs = 0x08;
    idt[num].flags = flags;
}

void idt_init()
{
    idtPtr.size = (sizeof(idt_entry_t) * 256) - 1; // it is an index of the last element
    idtPtr.address = idt;
    memset(idt, 0, sizeof(idt_entry_t) * 256);
    isr_init();
    _idt_load(&idtPtr);
}