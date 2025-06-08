#include <stdint.h>
#include <stddef.h>
#include "i686/drivers/screen/vga.h"
#include "printf.h"
#include "i686/drivers/serial/serial.h"
#include "i686/gdt.h"
#include "i686/idt.h"
#include "i686/isr.h"
#include "i686/irq.h"
#include "i686/io.h"
#include "i686/drivers/pic/pic.h"
#include "i686/drivers/keyboard/keyboard.h"

void kstart(void);



void halt()
{
    for (;;)
    {
        asm("hlt");
    }
}

void main_menu()
{
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

void kstart()
{
    VGA_clear(VGA_DEFAULT_COLOR);
    VGA_enable_cursor_blinking();

    printf("[+] Trying to initialise serial\n");
    int status = serial_init(SERIAL_COM1_BASE);
    if (status != 0)
    {
        printf("[-] Serial not initialised. Error code: %d\n", status);
        printf("[!] Halting kernel\n");
        goto halt;
    }

    printf("[+] Serial device COM1 initialised\n");
    serial_printf("[+] Serial device COM1 initialised\n");

    printf("[+] Trying to initialise gdt\n");
    gdt_install();
    check_gdt_loaded();
    printf("[+] GDT initialised, details sent to serial\n");
    printf("[+] Initialising Programable Interrupt Controller\n");
    PIC_configure(0x20, 0x28);
    printf("[+] Registering interrupt handlers\n");
    idt_install();
    isr_install();
    irq_install();
    irq_register_handler(1, keyboard_irq_handler);
    io_enableInterrupts();
    printf("[+] Testing interrupts...\n\n");
    asm("int $0x73");
    asm("int $0x72");
    asm("int $0x71");
    asm("int $0x70");


halt:
    halt();
}
