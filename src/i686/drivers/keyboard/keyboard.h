#pragma once
#include <stdint.h>
#include "i686/irq.h"

void keyboard_init();

void keyboard_irq_handler();

/** Temporary basic function that waits for keyboard input until a newline
*   @param buffer Typed ascii characters are stored here
*   @param length Size of the buffer
**/
void keyboard_get_line(char *buffer, size_t length);