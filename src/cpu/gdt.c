#include "gdt.h"
#include "utils/klog.h"

#define GDT_ENTRIES_COUNT 5

static gdt_entry gdt[GDT_ENTRIES_COUNT];
static gdt_descriptor gdt_ptr;

void _gdt_flush(gdt_descriptor *gdtptr);

static void gdt_set_entry(uint16_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);

    gdt[num].access_byte      = access;
}


void gdt_init()
{
    gdt_ptr.size    = (sizeof(gdt_entry) * GDT_ENTRIES_COUNT) - 1;   // it is an index of the last element
    gdt_ptr.address = gdt;

    gdt_set_entry(0, 0, 0,       0,    0   );  // null
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xA0);  // kernel code
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC0);  // kernel data
    gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0xA0);  // user code
    gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0xC0);  // user data

    klog_debug("GDT address: 0x%llx  size: %u", gdt_ptr.address, gdt_ptr.size);

    _gdt_flush(&gdt_ptr);
}