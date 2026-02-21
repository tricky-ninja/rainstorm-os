#include "pit.h"
#include "arch/i686/irq.h"

void pit_init(IRQHandler handler)
{
  irq_register_handler(0,handler);
}