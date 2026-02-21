#include <stdint.h>
#include <stddef.h>
#include "drivers/x86/screen/vga.h"
#include "printf.h"
#include "drivers/x86/serial/serial.h"
#include "arch/i686/gdt.h"
#include "arch/i686/idt.h"
#include "arch/i686/isr.h"
#include "arch/i686/irq.h"
#include "arch/i686/io.h"
#include "drivers/x86/pic/pic.h"
#include "drivers/x86/keyboard/keyboard.h"
#include <stdbool.h>
#include "memory_utils.h"
#include "drivers/x86/timer/pit.h"
#include "multiboot.h"
#include "memory/physical_memory.h"
#include "klog.h"

extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

static phys_addr_t kernel_start_addr;
static phys_addr_t kernel_end_addr;

void kstart(uint32_t magic, multiboot_info *mb_info);

static void early_init(multiboot_info *mb_info);
static void arch_init();
static void memory_init(multiboot_info *mb_info);
static void device_init();

void halt()
{
    for (;;)
    {
        asm("hlt");
    }
}


void kstart(uint32_t magic, multiboot_info *mb_info)
{
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        goto halt; // not properly booted


    early_init(mb_info);
    arch_init();
    memory_init(mb_info);
    device_init();
    shell_init();
    if (!strcmp(mb_info->cmdline, "debug=true"))
    {
        printf("Press enter to continue...\n");
        keyboard_get_line(NULL, 0);
    }

    vga_clear();
    shell_launch("prompt", mb_info);

halt:
    halt();
}


static void early_init(multiboot_info *mb_info)
{
    vga_set_color(VGA_DEFAULT_COLOR);
    vga_clear();
    vga_enable_cursor_blinking();
    printf("Hello from RainstormOS :)\n\n");

    int status = serial_init(SERIAL_COM1_BASE);
    if (status != 0) printf("[-] Serial not initialised\n");

    if (!strcmp(mb_info->cmdline, "debug=true"))
    {
        klog_set_debug(true);
        klog_set_level(klog_level_debug);
    }
    else if (!strcmp(mb_info->cmdline, "release=true"))
    {
        klog_set_debug(false);
        klog_set_level(klog_level_info);
    }
    else
    {
        klog_set_debug(true);
        klog_set_level(klog_level_info);
    }

    klog_debug("Multiboot info addr=0x%x", mb_info);
}

static void arch_init()
{
    gdt_install();
    dump_gdt_details();
    klog_info("GDT installed");


    idt_install(); // Set up and load the interupt discriptor table
    isr_install(); // Set up the interupt service routines (callback functions)
    irq_install(); // Set up hardware pic interupts and remaps pic to avoid cpu exception range

    klog_info("IDT and PIC configured");
}

static void memory_init(multiboot_info *mb_info)
{
    kernel_start_addr = &_kernel_start;
    kernel_end_addr = &_kernel_end;

    klog_debug("kernel_start=0x%x", kernel_start_addr);
    klog_debug("kernel_end=0x%x", kernel_end_addr);
    pmm_init(mb_info, kernel_start_addr, kernel_end_addr);
    klog_info("Total memory: %uKB", pmm_get_total_size() / 1024);
    klog_info("Total usable memory: %uKB", pmm_get_free_size() / 1024);

}

static void device_init()
{
    // Disable the programable interval timer for now
    pit_init(NULL);
    pic_setMask(0);

    keyboard_init();
    io_enableInterrupts();
}
