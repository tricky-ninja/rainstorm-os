#include "shell/shell.h"
#include "drivers/x86/screen/vga.h"

static shell_module_handler_fn handler(char *args, multiboot_info *mb_info)
{
    vga_clear();
    return 0;
}

SHELL_MODULE_REGISTER(
    .name = "clear",
    .handler = handler,
    .desc = "Clears the screen"
);
