#include "isr.h"
#include "printf.h"

void isr_handler(Registers *regs)
{
    printf("Got interrupt %d\n", regs->interrupt_num);
    serial_printf("Got interrupt %d\n", regs->interrupt_num);
}

