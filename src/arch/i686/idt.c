#include "idt.h"
#include "memory_utils.h"

idt_entry idt[256];
idt_descriptor idtPtr;

void idt_set_gate(uint8_t num, uint32_t offset, uint16_t segment, uint8_t flags)
{
    idt[num].offset_low = offset & 0xFFFF;
    idt[num].offset_high = (offset >> 16) & 0xFFFF;
    idt[num].segment_selector = segment;
    idt[num].flags = flags;
}

void idt_install()
{
    idtPtr.size = (sizeof(idt_entry) * 256) - 1;
    idtPtr.address = idt;

    memset(&idt, 0, sizeof(idt_entry) * 256);
    _idt_load();
}