#include <stdint.h>

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
    uint8_t *videoMemory = (uint8_t *)0xB8000;
    videoMemory[1] = 'H';
    // VGA_init();
    halt();
}