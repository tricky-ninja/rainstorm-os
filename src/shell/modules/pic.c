#include "shell/shell.h"
#include "drivers/x86/screen/vga.h"
#include "printf.h"
#include "memory_utils.h"
#include "drivers/x86/keyboard/keyboard.h"

static shell_module_handler_fn handler(char *args, multiboot_info *mb_info)
{
    if (args == NULL)
    {
        printf("Usage: pic disable\n");
        return 0;
    }
    strsplit(args, " ");
    if (strcmp(args, "disable")) 
    {
        printf("Unrecognised argument %s\n", args);
        printf("Usage: pic disable\n");
        return 0;
    }
    printf("WARNING! Running this command will make the os disable hardware interupts and it cant be enabled till reboot\nType 'yes' to confirm: ");
    keyboard_get_line(args, 256);
    if (strcmp(args, "yes"))
        return 0; // if not yes continue the loop
    pic_disable();
    return 0;
}

SHELL_MODULE_REGISTER(
        .name = "pic",
        .handler = handler,
        .desc = "Module for managing programable interupt controller");
