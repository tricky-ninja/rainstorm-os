#include "shell/shell.h"
#include "printf.h"

static int handler(const char *args, multiboot_info *mb_info)
{
    (void)args;
    (void)mb_info;

    printf("\n");
    printf("                    .-.\n");
    printf("                   (   ).\n");
    printf("                  (___(__)\n");
    printf("                   ' ' ' '\n");
    printf("                  RainstormOS\n");
    printf("\n");
    printf("              Kernel Shell Environment\n");
    printf("              Developed by TrickyNinja\n");
    printf("              (C) 2026 All Rights Reserved\n");
    printf("\n");
    printf("--------------------------------------------------\n");
    printf("Type 'help' to list available commands.\n\n");

    return 0;
}

SHELL_MODULE_REGISTER(
    .name = "prompt",
    .handler = handler,
    .desc = "Displays shell banner"
);