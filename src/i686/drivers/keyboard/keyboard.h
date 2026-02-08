#pragma once
#include <stdint.h>
#include "i686/irq.h"

void keyboard_init();

void keyboard_irq_handler();

void keyboard_get_line(char *buffer, size_t length);