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
#include <stdbool.h>
#include "memory.h"
#include "i686/drivers/timer/pit.h"

void kstart(void);



void halt()
{
    for (;;)
    {
        asm("hlt");
    }
}

IRQHandler timer_handler(Registers *regs)
{
    printf(".");
}

void main_menu()
{
    VGA_set_color(VGA_HACKER_COLOR);
    VGA_clear();

    printf("+----------------------------------------------------------+\n");
    printf("|              Rainstorm OS [Version 0.1.0]                |\n");
    printf("|       (c) 2026 TrickyNinja All rights reserved.          |\n");
    printf("|                                                          |\n");
    printf("|   > Welcome to Rainstorm OS                              |\n");
    printf("|   > Type 'help' to get started                           |\n");
    printf("+----------------------------------------------------------+\n\n");
    char cmd[256];
    while (true)
    {
        printf("$ ");
        keyboard_get_line(cmd, 256);
        if (!strcmp(cmd, "help"))
        {
            printf("clear - clears screen\nprompt - displays welcome prompts\n");
        }
        else if (!strcmp(cmd, "clear"))
        {
            VGA_clear();
        }
        else if (!strcmp(cmd, "pic disable"))
        {
            printf("WARNING! Running this command will make the os disable hardware interupts and it cant be enabled till reboot\nType 'yes' to confirm: ");
            keyboard_get_line(cmd, 256);
            if (strcmp(cmd, "yes")) continue; // if not yes continue the loop
            PIC_disable();
        }
        else if (!strcmp(cmd, "prompt"))
        {
            printf("+----------------------------------------------------------+\n");
            printf("|              Rainstorm OS [Version 0.1.0]                |\n");
            printf("|       (c) 2026 TrickyNinja All rights reserved.          |\n");
            printf("|                                                          |\n");
            printf("|   > Welcome to Rainstorm OS                              |\n");
            printf("|   > Type 'help' to get started                           |\n");
            printf("+----------------------------------------------------------+\n\n");
        }
    }
}

void kstart()
{
    VGA_set_color(VGA_DEFAULT_COLOR);
    VGA_clear();
    VGA_enable_cursor_blinking();

    printf("Hello from Rainstorm OS\n");
    printf("[+] Trying to initialise serial\n");
    int status = serial_init(SERIAL_COM1_BASE);
    if (status != 0)
    {
        printf("[-] Serial not initialised. Error code: %d\n", status);
    }

    else {
        printf("[+] Serial device COM1 initialised\n");
        serial_printf("[+] Serial device COM1 initialised\n");
    }

    printf("[+] Trying to initialise gdt\n");
    gdt_install();
    check_gdt_loaded();
    printf("[+] GDT initialised, details sent to serial\n");
    printf("[+] Initialising Programable Interrupt Controller\n");


    PIC_configure(0x20, 0x28);
    printf("[+] Registering interrupt handlers\n");
    idt_install();  // Set up and load the interupt discriptor table
    isr_install();  // Set up the interupt service routines (callback functions)
    irq_install();  // Set up hardware pic interupts

    pit_init(timer_handler);   // Enable the hardware programmable interval timer
    PIC_setMask(0); // Disable timer interupt
    keyboard_init();    // Configure the keyboard driver
    io_enableInterrupts();  // Enable interupts

    printf("Press enter to continue...\n");
    keyboard_get_line(NULL, 0);
    main_menu();

halt:
    halt();
}
