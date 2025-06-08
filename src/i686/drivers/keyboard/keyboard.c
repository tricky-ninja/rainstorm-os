#include "keyboard.h"
#include "i686/io.h"
#include "printf.h"

void keyboard_irq_handler()
{
    uint8_t scancode = read_portb(0x60);
    printf("%d ", scancode);
}