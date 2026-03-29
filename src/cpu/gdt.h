#pragma once
#include <stddef.h>
#include <stdint.h>

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


void gdt_init();



