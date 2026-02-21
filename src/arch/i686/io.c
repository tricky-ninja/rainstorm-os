#include "io.h"

uint8_t read_portb(uint16_t port)
{
  uint8_t result;
  __asm__("in %%dx, %%al"
          : "=a"(result)
          : "d"(port));
  return result;
}

void write_portb(uint16_t port, uint8_t data)
{
  __asm__("out %%al, %%dx"
          : : "a"(data)
          , "d"(port));
}

void io_wait()
{
  write_portb(0x80, 0);   // write to an unused port
}

void io_enableInterrupts()
{
  asm("sti");
}

void io_disableInterrupts()
{
  asm("cli");
}