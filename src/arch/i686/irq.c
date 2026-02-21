/**
 *  IRQ Implementation
 *
 *  Handles hardware interrupts (mapped to 0x20–0x2F).
 *
 *  Requires PIC to be remapped before use.
 *
 *  Sends End-Of-Interrupt (EOI) to PIC after handling.
 *  Author: Sreyas A (TrickyNinja)
 */

#include "irq.h"
#include "isr.h"
#include "io.h"
#include "drivers/x86/pic/pic.h"
#include "printf.h"
#include "klog.h"


#define PIC_REMAP_OFFSET        0x20

IRQHandler g_IRQHandlers[16];

void irq_handler(Registers *regs)
{
    if (g_IRQHandlers[regs->interrupt_num - 0x20] == NULL) klog_critical("Unhandled IRQ %d recieved", regs->interrupt_num);

    else
    {
        g_IRQHandlers[regs->interrupt_num - 0x20](regs);
    }

    if (regs->interrupt_num > 0x28)
        write_portb(SLAVE_PIC_COMMAND, 0x20);
    write_portb(MASTER_PIC_COMMAND, 0x20);
}

void irq_install()
{
    pic_configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 0x8);

    for (uint8_t i =0; i < 16; i++)
        isr_register_handler(PIC_REMAP_OFFSET + i, irq_handler);

}

void irq_register_handler(uint8_t irq, IRQHandler handler)
{
    g_IRQHandlers[irq] = handler;
}