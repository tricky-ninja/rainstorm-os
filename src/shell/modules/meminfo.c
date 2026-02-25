#include "shell/shell.h"
#include "drivers/x86/screen/vga.h"
#include "printf.h"

static shell_module_handler_return_t handler(char *args, multiboot_info *mb_info)
{
    (void)args;

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
    return 0;
}

SHELL_MODULE_REGISTER(
        .name = "meminfo",
        .handler = handler,
        .desc = "Display memory map");
