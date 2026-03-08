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
#include "shell/shell.h"
#include <stddef.h>

extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

static uint8_t *kernel_start_addr_virt;
static uint8_t *kernel_end_addr_virt;

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
    if (!strcmp((char*)mb_info->cmdline, "debug=true"))
    {
        printf("Press any key to continue...\n");
        keyboard_get_char();
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

    if (!strcmp((char*)mb_info->cmdline, "debug=true"))
    {
        klog_set_debug(true);
        klog_set_level(klog_level_debug);
    }
    else if (!strcmp((char*)mb_info->cmdline, "release=true"))
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
    kernel_start_addr_virt = &_kernel_start;
    kernel_end_addr_virt = &_kernel_end;

    klog_debug("kernel_start=0x%x", kernel_start_addr_virt);
    klog_debug("kernel_end=0x%x", kernel_end_addr_virt);

    pmm_range_t *ranges = (pmm_range_t *)(uintptr_t)PMM_ALIGN_UP((uintptr_t)kernel_end_addr_virt);
    size_t ranges_count = multiboot_mmap_parse((uint8_t*)mb_info->mmap_addr, mb_info->mmap_length, ranges);
    pmm_range_t *ranges_end = &ranges[ranges_count];

    uint8_t *bitmap_addr_virt = (uint8_t *)(uintptr_t)PMM_ALIGN_UP((uintptr_t)ranges_end);
    phys_addr_t bitmap_addr_physical = (phys_addr_t)(uintptr_t)bitmap_addr_virt;    // No paging for now

    pmm_init(ranges, ranges_count, bitmap_addr_virt, bitmap_addr_physical, (phys_addr_t)(uintptr_t)kernel_start_addr_virt, (phys_addr_t)(uintptr_t)kernel_end_addr_virt);

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

size_t multiboot_mmap_parse(uint8_t *ptr, size_t length, pmm_range_t *ranges)
{
    uint8_t *end = ptr + length;
    size_t i = 0;
    size_t highest_usable_index = 0;
    while (ptr < end)
    {
        multiboot_mmap_entry *entry = (multiboot_mmap_entry*)ptr;
        phys_addr_t start_addr = entry->addr;
        phys_addr_t end_addr = entry->addr + entry->len;
        pmm_memory_type type = entry->type == MULTIBOOT_MEMORY_AVAILABLE ? PMM_FREE : PMM_ALLOCATED;

        if (start_addr >= PMM_LIMIT_32)
        {
            ptr += entry->size + sizeof(entry->size);
            continue;
        }

        if (end_addr >= PMM_LIMIT_32)
        {
            end_addr = PMM_LIMIT_32;
        }

        if (end_addr <= start_addr)
        {
            ptr += entry->size + sizeof(entry->size);
            continue;
        }

        ranges[i].start = start_addr;
        ranges[i].end = end_addr;
        ranges[i].type = type;

        ptr += entry->size + sizeof(entry->size);
        i++;
        if (type == PMM_FREE) highest_usable_index = i;
    }

    return highest_usable_index;

}
