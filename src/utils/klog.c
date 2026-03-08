#include "klog.h"
#include <stdbool.h>
#include "drivers/x86/screen/vga.h"

static const char *const log_labels[] = {
    [klog_level_debug] = "[debug]: ",
    [klog_level_info] = "[info]: ",
    [klog_level_warn] = "[warn]: ",
    [klog_level_error] = "[ERROR]: ",
    [klog_level_critical] = "[CRITICAL]: ",
};

static klog_Level current_level = klog_level_info;
bool is_debug = false;

void klog_set_level(klog_Level level)
{
    current_level = level;
}

void klog_set_debug(bool val)
{
    is_debug = val;
}

void klogf(klog_Level level, char *string, ...)
{
    if (level < current_level)
        return;

    if (level == klog_level_critical)
    {
        asm volatile("cli");
        serial_printf("PANIC!!!");
        vga_set_color(VGA_PANIC_COLOR);
        vga_clear();
        printf("\n");
        printf("        .-.\n");
        printf("       (   ).\n");
        printf("      (___(__)\n");
        printf("\n");
        printf("      ( x_x )\n");
        printf("\n");
        printf("    !!KERNEL PANIC!!\n");
        printf("\n");

        va_list args;
        va_start(args, string);

        serial_printf(log_labels[level]);
        serial_vprintf(string, args);
        serial_printf("\n");

        va_list args_copy;
        va_copy(args_copy, args);
        vprintf(string, args_copy);
        printf("\n");
        va_end(args_copy);
        serial_printf("kernel halted");
        printf("kernel halted");
        asm volatile("hlt");
    }

    va_list args;
    va_start(args, string);

    serial_printf(log_labels[level]);
    serial_vprintf(string, args);
    serial_printf("\n");

    if (is_debug)
    {
        va_list args_copy;
        va_copy(args_copy, args);
        printf(log_labels[level]);
        vprintf(string, args_copy);
        printf("\n");
        va_end(args_copy);
    }

    va_end(args);
}