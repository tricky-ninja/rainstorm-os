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
#include "multiboot.h"
#include "memory/physical_manager.h"

extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

void kstart(uint32_t magic, multiboot_info *);

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

void main_menu(multiboot_info *mb_info)
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
            printf("clear - clears screen\nprompt - displays welcome prompts\nbootinfo - displays information about the system\nmeminfo - display memory map\nmemtest - tests the physical memory manager\n");
        }
        else if (!strcmp(cmd, "clear"))
        {
            VGA_clear();
        }
        else if (!strcmp(cmd, "pic disable"))
        {
            printf("WARNING! Running this command will make the os disable hardware interupts and it cant be enabled till reboot\nType 'yes' to confirm: ");
            keyboard_get_line(cmd, 256);
            if (strcmp(cmd, "yes"))
                continue; // if not yes continue the loop
            PIC_disable();
        }
        else if (!strcmp(cmd, "bootinfo"))
        {
            printf("\nFlags: 0x%x\n", mb_info->flags);
            printf("Lower memory: 0x%x\n", mb_info->mem_lower);
            printf("Upper memory: 0x%x\n", mb_info->mem_upper);
            printf("Boot device: 0x%x\n", mb_info->boot_device);
            if (mb_info->flags & (1 << 2))
                printf("Cmdline: %s\n", (char *)mb_info->cmdline);
            printf("Module count: 0x%x\n", mb_info->mods_count);
            printf("Modules address: 0x%x\n", mb_info->mods_addr);
            printf("Memory map length: 0x%x\n", mb_info->mmap_length);
            printf("Memory map address: 0x%x\n", mb_info->mmap_addr);
            if (mb_info->flags & (1 << 9))
                printf("Bootloader: %s\n\n", (char *)mb_info->boot_loader_name);
        }
        else if (!strcmp(cmd, "meminfo"))
        {
            uint8_t *ptr = (uint8_t *)mb_info->mmap_addr;
            uint8_t *end = ptr + mb_info->mmap_length;

            serial_printf("\nMemory map:\n");
            while (ptr < end)
            {
                multiboot_mmap_entry *entry = (multiboot_mmap_entry *)ptr;
                uint64_t start = entry->addr;
                uint64_t end = entry->addr + entry->len;
                uint32_t type = entry->type;
                char type_char = 'R';
                if (type == MULTIBOOT_MEMORY_AVAILABLE)
                    type_char = 'A';
                printf("0x%x%08x to 0x%x%08x\t-\t%u(%c), Size: %uKB\n",
                       (uint32_t)(start >> 32),
                       (uint32_t)start,
                       (uint32_t)(end >> 32),
                       (uint32_t)end,
                       type, type_char,
                       (uint32_t)(entry->len / 1024));

                serial_printf("0x%x%08x to 0x%x%08x\t-\t%u(%c), Size: %uKB\n",
                              (uint32_t)(start >> 32),
                              (uint32_t)start,
                              (uint32_t)(end >> 32),
                              (uint32_t)end,
                              type, type_char,
                              (uint32_t)(entry->len / 1024));
                ptr += entry->size + sizeof(entry->size);
            }
        }

        else if (!strcmp(cmd, "memtest"))
        {
            printf("Total pages: %u\n", pmm_get_total_size() / PAGE_SIZE);
            printf("Total RAM: %u MB\n", pmm_get_total_size() / 1024 / 1024);
            printf("Kernel free: %u (expected 0)\n", pmm_is_region_free(_kernel_start));
            printf("Lower region free: %u (expected 0)\n", pmm_is_region_free(4096));
            printf("Must be free: %u (expected 1)\n", pmm_is_region_free(PMM_ALIGN_UP(10 * 1024 * 1024)));
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
        else
        {
            printf("%s is not a valid command\n", cmd);
        }
    }
}

void kstart(uint32_t magic, multiboot_info *mb_info)
{
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        goto halt; // not properly booted

    VGA_set_color(VGA_DEFAULT_COLOR);
    VGA_clear();
    VGA_enable_cursor_blinking();

    printf("Magic=0x%x\n", magic);
    printf("Hello from Rainstorm OS\n");
    printf("[+] Trying to initialise serial\n");
    int status = serial_init(SERIAL_COM1_BASE);
    if (status != 0)
    {
        printf("[-] Serial not initialised. Error code: %d\n", status);
    }

    else
    {
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
    idt_install(); // Set up and load the interupt discriptor table
    isr_install(); // Set up the interupt service routines (callback functions)
    irq_install(); // Set up hardware pic interupts

    pit_init(timer_handler); // Enable the hardware programmable interval timer
    PIC_setMask(0);          // Disable timer interupt
    keyboard_init();         // Configure the keyboard driver
    io_enableInterrupts();   // Enable interupts

    printf("kernel_start=0x%x\nkernel_end=0x%x\n", &_kernel_start, &_kernel_end);
    serial_printf("kernel_start=0x%x\nkernel_end=0x%x\n", &_kernel_start, &_kernel_end);

    printf("\n");

    pmm_init(mb_info, _kernel_start, _kernel_end);
    printf("Total memory: %uKB\n", pmm_get_total_size() / 1024);
    uint32_t total_mem = (mb_info->mem_lower + mb_info->mem_upper) * 1024;
    total_mem = PMM_ALIGN_UP(total_mem);
    printf("Total memory (grub): %uKB\n", total_mem / 1024);

    printf("Press enter to continue...\n");
    keyboard_get_line(NULL, 0);
    main_menu(mb_info);

halt:
    halt();
}
