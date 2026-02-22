#include "shell/shell.h"
#include "printf.h"

static int handler(const char *args, multiboot_info *mb_info)
{
    (void)args;
    (void)mb_info;

printf("\n");
printf("        .-.\n");
printf("       (   ).\n");
printf("      (___(__)\n");
printf("       ' ' ' '\n");
printf("\n");
printf("        RainstormOS\n");
printf("        (c) 2026 TrickyNinja. All rights reserved.\n");
printf("\n");
printf("--------------------------------------------------\n");
printf(" Kernel Shell Environment\n");
printf(" Type 'help' to list available commands.\n");
printf("--------------------------------------------------\n\n");

    return 0;
}

SHELL_MODULE_REGISTER(
        .name = "prompt",
        .handler = handler,
        .desc = "Displays shell banner");