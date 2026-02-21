#pragma once
#include "printf.h"
#include <stdbool.h>

#define klog_debug(...) klogf(klog_level_debug,  __VA_ARGS__)
#define klog_info(...) klogf(klog_level_info, __VA_ARGS__)
#define klog_warn(...) klogf(klog_level_warn,  __VA_ARGS__)
#define klog_error(...) klogf(klog_level_error, __VA_ARGS__)
#define klog_critical(...) klogf(klog_level_critical, __VA_ARGS__)

typedef enum
{
    klog_level_debug,
    klog_level_info,
    klog_level_warn,
    klog_level_error,
    klog_level_critical
} klog_Level;

void klog_set_level(klog_Level level);
void klogf(klog_Level level, char *string, ...);
void klog_set_debug(bool val);