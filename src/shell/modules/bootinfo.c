#include "shell/shell.h"
#include "drivers/x86/screen/vga.h"
#include "printf.h"

static shell_module_handler_fn handler(char *args, multiboot_info *mb_info)
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
    return 0;
}

SHELL_MODULE_REGISTER(
        .name = "bootinfo",
        .handler = handler,
        .desc = "Displays information about the system");
