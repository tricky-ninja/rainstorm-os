#include "i686/irq.h"
#include "i686/isr.h"
#include "i686/io.h"
#include "i686/drivers/pic/pic.h"
#include "printf.h"


#define PIC_REMAP_OFFSET        0x20

IRQHandler g_IRQHandlers[16];

void timer(Registers *regs)
{
    printf(".");
}

void irq_handler(Registers *regs)
{
    if (g_IRQHandlers[regs->interrupt_num - 0x20] == NULL) printf("Unhandled IRQ %d recieved\n", regs->interrupt_num);

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
    PIC_configure(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 0x8);

    for (uint8_t i =0; i < 16; i++)
        isr_register_handler(PIC_REMAP_OFFSET + i, irq_handler);

    irq_register_handler(0, timer);
    PIC_setMask(0);
}

void irq_register_handler(uint8_t irq, IRQHandler handler)
{
    g_IRQHandlers[irq] = handler;
}