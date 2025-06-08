#pragma once
#include <stdint.h>
#include "i686/isr.h"

typedef void (*IRQHandler)(Registers* regs);

void irq_install();
void irq_register_handler(uint8_t irq, IRQHandler handler);