#pragma once

#include <stdint.h>

#define MASTER_PIC_COMMAND 0x20
#define MASTER_PIC_DATA 0x21

#define SLAVE_PIC_COMMAND 0xA0
#define SLAVE_PIC_DATA 0xA1

#define PIC_EOI

typedef enum PIC_ICW1
{
    PIC_ICW1_ICW4   = 0x1,
    PIC_ICW1_SINGLE = 0x2,
    PIC_ICW1_LEVEL  = 0x8,
    PIC_ICW1_INIT   = 0x10,
} PIC_ICW1;

void PIC_configure(uint8_t offset1, uint8_t offset2);

void PIC_setMask(uint8_t irq);
void PIC_clearMask(uint8_t irq);

void PIC_disable();
void PIC_enable();