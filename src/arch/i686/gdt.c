/**
 *  GDT Implementation
 *
 *  Creates flat memory model:
 *      Base = 0x0
 *      Limit = 4GB
 *
 *  Segments:
 *      - 0x08: Kernel code
 *      - 0x10: Kernel data
 *
 *  Assumes no user-mode support yet.
 *  Author: Sreyas A (TrickyNinja)
 */

#include "gdt.h"
#include "klog.h"

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

    klog_debug("Address of gdt: 0x%08x\n", gdtPtr.address);

    gdt_set_gate(0,0,0,0,0);

    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    _gdt_flush();
}

void dump_gdt_details(void) {
    gdt_descriptor current;
    // Copy GDTR into our struct
    asm volatile("sgdt %0" : "=m"(current));

    // Now print out what we got:
    printf("\n");
    serial_printf("\n");
    klog_debug("GDTR.limit = 0x%04x", current.size);
    klog_debug("GDTR.base  = 0x%08x", current.address);

    // Check segment registers:
    uint16_t cs, ds, es, fs, gs, ss;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    asm volatile("mov %%ds, %0" : "=r"(ds));
    asm volatile("mov %%es, %0" : "=r"(es));
    asm volatile("mov %%fs, %0" : "=r"(fs));
    asm volatile("mov %%gs, %0" : "=r"(gs));
    asm volatile("mov %%ss, %0" : "=r"(ss));

    klog_debug("CS = 0x%04x", cs);
    klog_debug("DS = 0x%04x", ds);
    klog_debug("ES = 0x%04x", es);
    klog_debug("FS = 0x%04x", fs);
    klog_debug("GS = 0x%04x", gs);
    klog_debug("SS = 0x%04x\n", ss);
}