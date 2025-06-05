#include "gdt.h"
#include "printf.h"

gdt_entry gdt[GDT_ENTRIES_COUNT];
gdt_descriptor gdtPtr;

void gdt_set_gate(uint16_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].base_low = (base & 0xffff);
    gdt[num].base_middle = ((base >> 16) & 0xFF);
    gdt[num].base_high = ((base >> 24) & 0xFF);

    gdt[num].limit_low = (limit & 0xffff);
    gdt[num].granularity = ((limit >> 16) & 0xffff);

    gdt[num].granularity |= (gran & 0xf0);
    gdt[num].access_byte = access;
}

void gdt_install()
{
    gdtPtr.size = (sizeof(gdt_entry) * GDT_ENTRIES_COUNT) - 1;
    gdtPtr.address = gdt;

    serial_printf("Address of gdt: 0x%08x\n", gdtPtr.address);

    gdt_set_gate(0,0,0,0,0);

    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    _gdt_flush();
}

void check_gdt_loaded(void) {
    gdt_descriptor current;
    // Copy GDTR into our struct
    asm volatile("sgdt %0" : "=m"(current));

    // Now print out what we got:
    serial_printf("GDTR.limit = 0x%04x\n", current.size);
    serial_printf("GDTR.base  = 0x%08x\n", current.address);

    // Check segment registers:
    uint16_t cs, ds, es, fs, gs, ss;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    asm volatile("mov %%ds, %0" : "=r"(ds));
    asm volatile("mov %%es, %0" : "=r"(es));
    asm volatile("mov %%fs, %0" : "=r"(fs));
    asm volatile("mov %%gs, %0" : "=r"(gs));
    asm volatile("mov %%ss, %0" : "=r"(ss));

    serial_printf("CS = 0x%04x\n", cs);
    serial_printf("DS = 0x%04x\n", ds);
    serial_printf("ES = 0x%04x\n", es);
    serial_printf("FS = 0x%04x\n", fs);
    serial_printf("GS = 0x%04x\n", gs);
    serial_printf("SS = 0x%04x\n", ss);
}