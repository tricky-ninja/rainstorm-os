/**
 *  Keyboard Driver Implementation
 *
 *  Interrupt-driven input using IRQ1.
 *  Basic US keymap.
 *
 *  Provides simple line-buffered input for shell.
 *  Author: Sreyas A (TrickyNinja)
 */

#include "keyboard.h"
#include "arch/i686/io.h"
#include "printf.h"
#include <stdbool.h>
#include "memory_utils.h"
#include "drivers/x86/pic/pic.h"

static char kbd_buf[KBD_BUF_SIZE];
static volatile size_t kbd_head = 0;
static volatile size_t kbd_tail = 0;

const char scancode_to_char[] = {
    '?', '?', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    '\b', '?', 'q', 'w', 'e', 'r', 't',
    'y', 'u', 'i', 'o', 'p', '[', ']',
    '\n', '?', 'a', 's', 'd', 'f', 'g',
    'h', 'j', 'k', 'l', ';', '\\', '`',
    '?', '\\', 'z', 'x', 'c', 'v', 'b',
    'n', 'm', ',', '.', '/', '?', '?',
    '?', ' '};

void keyboard_init()
{
    memset(kbd_buf, '\0', KBD_BUF_SIZE);
    kbd_head = 0;
    kbd_tail = 0;
    irq_register_handler(1, keyboard_irq_handler);
}

void keyboard_irq_handler()
{
    uint8_t scancode = read_portb(0x60);
    if (scancode > 57)
        return;

    char ch = scancode_to_char[scancode];
    if (!ch)
        return;

    size_t next = (kbd_head + 1) % KBD_BUF_SIZE;

    if (next == kbd_tail)
        return;
    kbd_buf[kbd_head] = ch;
    kbd_head = next;
}

char keyboard_get_char()
{
    while (kbd_tail == kbd_head)
    {
        asm volatile("hlt");
    }


    char ch = kbd_buf[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUF_SIZE;

    return ch;
}

void keyboard_get_line(char *buffer, size_t length)
{
    if (buffer != NULL && length < 2) return;
    size_t i = 0;

    // If length is 0 then this will loop till new line (this case will only be reached if buffer is NULL so in this case its just echo everything but dont store)
    while (i < length - 1)
    {
        char ch = keyboard_get_char();

        if (ch == '\b')
        {
            if (i > 0)
            {
                i--;
                printf("%c", ch); 
            }
            continue;
        }

        printf("%c", ch);

        if (ch == '\n')
            break;

        if (buffer != NULL) buffer[i++] = ch;
    }

    if (buffer != NULL) buffer[i] = '\0';
}