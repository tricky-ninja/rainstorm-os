#include "shell/shell.h"
#include "drivers/x86/screen/vga.h"
#include "printf.h"
#include "memory/physical_memory.h"
#include "drivers/x86/keyboard/keyboard.h"
#include "memory_utils.h"

static shell_module_handler_return_t handler(char *args, multiboot_info *mb_info)
{
    (void)mb_info;

    if (args == NULL)
    {
        printf("Usage: pmm <allocate/stats/test>\n");
        return 0;
    }
    strsplit(args, ' ');
    if (!strcmp(args, "allocate"))
    {
        phys_addr_t addr = pmm_alloc_page();
        printf("Allocated physical page at addr 0x%x\n", addr);
        return 0;
    }

    if (!strcmp(args, "stats"))
    {
        uint32_t total_pages = pmm_get_total_size() / PAGE_SIZE;
        uint32_t total_ram_mb = pmm_get_total_size() / 1024 / 1024;
        uint32_t free_ram_mb = pmm_get_free_size() / 1024 / 1024;
        uint32_t free_before = pmm_get_free_size() / PAGE_SIZE;

        printf("Total pages: %u\n", total_pages);
        printf("Free pages: %u\n", free_before);
        printf("Total RAM: %u MB\n", total_ram_mb);
        printf("Free RAM: %u MB\n", free_ram_mb);

        return 0;
    }

    if (!strcmp(args, "test"))
    {
        return shell_run("memtest", args, mb_info);
    }

    printf("Usage: pmm <allocate/stats/test>\n");

    return 0;
}

SHELL_MODULE_REGISTER(
        .name = "pmm",
        .handler = handler,
        .desc = "Module for managing physical memory");
