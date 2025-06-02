#include <stdint.h>
#include <stddef.h>
#include "vga.h"
#include "printf.h"
#include "serial.h"

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
    VGA_clear(VGA_DEFAULT_COLOR);
    VGA_enable_cursor_blinking();

    printf("Trying to initialise serial\n");
    int status = serial_init(SERIAL_COM1_BASE);
    if (status != 0) 
    {
        printf("Serial not initialised. Error code: %d\n", status);
        printf("Halting kernel\n");
        halt();
    }
    else
    {
        printf("Serial device COM1 initialised\n");
        serial_printf("Serial device COM1 initialised\n");
        VGA_clear(VGA_DEFAULT_COLOR);

        printf("+----------------------------------------------------------+\n");
        printf("|              Rainstorm OS [Version 0.1.0]                |\n");
        printf("|       (c) 2025 TrickyNinja All rights reserved.          |\n");
        printf("|                                                          |\n");
        printf("|   > Welcome to Rainstorm OS                              |\n");
        printf("|   > Type 'help' to get started                           |\n");
        printf("+----------------------------------------------------------+\n");
        printf("\n>");
    }
    halt();
}