/**
 *  Module: 8259 Programmable Interrupt Controller
 *
 *  Handles:
 *      - PIC remapping
 *      - IRQ masking/unmasking
 *
 *  Required before enabling hardware interrupts.
 *  Author: Sreyas A (TrickyNinja)
 */

#pragma once

#include <stdint.h>

// I/O port addresses for the master PIC
#define MASTER_PIC_COMMAND 0x20
#define MASTER_PIC_DATA    0x21

// I/O port addresses for the slave PIC
#define SLAVE_PIC_COMMAND  0xA0
#define SLAVE_PIC_DATA     0xA1

// End Of Interrupt command value
#define PIC_EOI 0x20

/**
 *  Initialization Command Word 1 (ICW1) flags
 *
 *  Used during PIC initialization sequence.
 */
typedef enum PIC_ICW1
{
    PIC_ICW1_ICW4   = 0x01,   // ICW4 will be sent
    PIC_ICW1_SINGLE = 0x02,   // Single (no slave)
    PIC_ICW1_LEVEL  = 0x08,   // Level triggered mode
    PIC_ICW1_INIT   = 0x10,   // Initialization command
} PIC_ICW1;


/**
 *  Configures and remaps the PIC.
 *
 *  Sends the full initialization control word (ICW) sequence
 *  to both master and slave PICs.
 *
 *  Remap IRQs away from CPU exception range
 *  (e.g., from 0x08–0x0F to 0x20–0x2F).
 *
 *  @param offset1 Interrupt vector offset for master PIC
 *  @param offset2 Interrupt vector offset for slave PIC
 */
void pic_configure(uint8_t offset1, uint8_t offset2);


/**
 *  Disables a specific IRQ line.
 *
 *  Prevents the specified hardware interrupt from being delivered.
 *
 *  @param irq IRQ number (0–15)
 */
void pic_setMask(uint8_t irq);

/**
 *  Enables a specific IRQ line.
 *
 *  Allows the specified hardware interrupt to be delivered.
 *
 *  @param irq IRQ number (0–15)
 */
void pic_clearMask(uint8_t irq);

void pic_disable();
void pic_enable();