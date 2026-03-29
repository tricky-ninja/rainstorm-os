#include "klog.h"
#include <stdbool.h>
#include "sink.h"
#include "drivers/serial/serial.h"
#include "drivers/screen/framebuffer.h"

static const char *const log_labels[] = {
    [klog_level_debug] = "[debug]: ",
    [klog_level_info] = "[info]: ",
    [klog_level_warn] = "[warn]: ",
    [klog_level_error] = "[ERROR]: ",
    [klog_level_panic] = "[PANIC]: ",
};

static klog_level current_level = klog_level_info;
static int debug_sink = -1;

static void debug_sink_write(char ch)
{
    serial_write(ch);
}

void klog_set_level(klog_level level)
{
    current_level = level;
    if (debug_sink < 0)
        debug_sink = sink_register(debug_sink_write);
}

void klogf(klog_level level, const char *string, ...)
{

    if (level < current_level)
        return;

    va_list args;
    va_start(args, string);
    
    // TODO: make a seperate panic handler
    if (level == klog_level_panic)
    {
        asm volatile("cli");
        kprintf_set_default_sink(debug_sink);
        fb_clear_screen((fb_color_t){
            .red = 235,
            .green = 52,
            .blue = 76});

        kprintf("\n--- KERNEL PANIC ---\n");
        kvprintf(string, args);
        kprintf("\n");

        uint64_t *rbp;
        asm volatile("mov %%rbp, %0" : "=r"(rbp));
        kprintf("Stack trace:\n");
        for (int i = 0; i < 16 && rbp; i++)
        {
            kprintf("  [%d] 0x%016llx\n", i, rbp[1]);
            rbp = (uint64_t *)rbp[0];
        }

        kprintf("--------------------\n");
         va_end(args);

        for (;;)
        {
            asm volatile("hlt");
        };
    }

     if (debug_sink < 0)
        return;


    int prev_sink = kprintf_get_default_sink();
    kprintf_set_default_sink(debug_sink);
    kprintf(log_labels[level]);
    kvprintf(string, args);
    kprintf("\n");
    kprintf_set_default_sink(prev_sink);
    va_end(args);

    
}