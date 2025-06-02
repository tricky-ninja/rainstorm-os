#include <stdint.h>
#include <stddef.h>
#include "vga.h"
#include "printf.h"

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
    VGA_init();
    VGA_clear(VGA_DEFAULT_COLOR);
    printf("It works: %d", 20);
    halt();
}