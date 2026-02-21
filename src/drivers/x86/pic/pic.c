/**
 *  PIC Implementation
 *
 *  Remaps IRQs to avoid overlap with CPU exceptions.
 *  Supports enabling/disabling specific IRQ lines.
 *
 *  Legacy hardware interface.
 *  Author: Sreyas A (TrickyNinja)
 */

#include "pic.h"
#include "arch/i686/io.h"

void pic_configure(uint8_t offset1, uint8_t offset2)
{
    write_portb(MASTER_PIC_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_wait();
    write_portb(SLAVE_PIC_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_wait();
    
    write_portb(MASTER_PIC_DATA, offset1);
    io_wait();
    write_portb(SLAVE_PIC_DATA, offset2);
    io_wait();

    write_portb(MASTER_PIC_DATA, 0x4);   // Tell the master pic that it has a slave on IRQ2
    io_wait();
    write_portb(SLAVE_PIC_DATA, 0x2);    // Tell slave pic its cascade id
    io_wait();

    write_portb(MASTER_PIC_DATA, 0x1);   // 8086 mode
    io_wait();
    write_portb(SLAVE_PIC_COMMAND, 0x1);
    io_wait();

    pic_enable();
}

void pic_enable()
{
    write_portb(MASTER_PIC_DATA, 0);
    io_wait();
    write_portb(SLAVE_PIC_DATA, 0);
    io_wait();
}

void pic_disable()
{
    write_portb(MASTER_PIC_DATA, 0xff);
    io_wait();
    write_portb(SLAVE_PIC_DATA, 0xff);
    io_wait();
}

void pic_setMask(uint8_t irq)
{
    uint16_t port;
    uint8_t value;

    if(irq < 8) {
        port = MASTER_PIC_DATA;
    } else {
        port = SLAVE_PIC_DATA;
        irq -= 8;
    }
    value = read_portb(port) | (1 << irq);
    write_portb(port, value);
}

void PIC_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if(irq < 8) {
        port = MASTER_PIC_DATA;
    } else {
        port = SLAVE_PIC_DATA;
        irq -= 8;
    }
    value = read_portb(port) & ~(1 << irq);
    write_portb(port, value);        
}