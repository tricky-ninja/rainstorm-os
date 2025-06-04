#pragma once
#include <stddef.h>
#include <stdint.h>

#define GDT_ENTRIES_COUNT 3

typedef struct gdt_ptr
{
    uint16_t size;
    size_t address;
} __attribute__((packed)) gdt_ptr;

typedef struct gdt_entry
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access_byte;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) gdt_entry;

extern gdt_entry gdt[GDT_ENTRIES_COUNT];
extern gdt_ptr gdtPtr;

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
void gdt_install();

void check_gdt_loaded();

extern void gdt_flush();


