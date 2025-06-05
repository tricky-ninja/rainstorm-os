#pragma once
#include <stddef.h>
#include <stdint.h>

#define GDT_ENTRIES_COUNT 3



typedef struct gdt_entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access_byte;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry;

typedef struct gdt_descriptor
{
    uint16_t size;
    gdt_entry *address;
} __attribute__((packed)) gdt_descriptor;


extern gdt_entry gdt[GDT_ENTRIES_COUNT];
extern gdt_descriptor gdtPtr;

void gdt_set_gate(uint16_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_install();

void check_gdt_loaded();

extern void _gdt_flush();


