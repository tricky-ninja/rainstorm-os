#pragma once

#include "multiboot.h"

typedef int shell_module_handler_return_t;
typedef shell_module_handler_return_t (*shell_module_handler_fn)(char *, multiboot_info *);

#define SHELL_MODULE_REGISTER(...) \
    static const shell_module __shell_module_##__LINE__ \
    __attribute__((section(".shell_modules"), used, aligned(__alignof__(shell_module)))) \
    = { __VA_ARGS__ }

typedef struct shell_module
{
    char name[256];
    shell_module_handler_fn handler;
    char desc[512];
} shell_module;

void shell_init();
void shell_launch(char *cmd, multiboot_info *mb_info);
