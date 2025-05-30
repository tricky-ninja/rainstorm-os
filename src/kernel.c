#include <stdint.h>
#include <stddef.h>
#include "vga.h"

void kstart(void);

void halt()
{
    for (;;)
    {
        asm("hlt");
    }
}

void kstart()
{
    const char *msg = "It works!!!";
    VGA_init();
    VGA_clear(VGA_DEFAULT_COLOR);
    size_t i =0;
    while (1)
    {
        i = i % 9;
        VGA_print_char('0'+i, VGA_DEFAULT_COLOR);
        // VGA_print_char('\n', VGA_DEFAULT_COLOR);
        i++;
    }
    halt();
}