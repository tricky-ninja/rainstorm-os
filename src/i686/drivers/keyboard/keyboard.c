#include "keyboard.h"
#include "i686/io.h"
#include "printf.h"
#include <stdbool.h>
#include "memory.h"
#include "i686/drivers/pic/pic.h"

bool isRecordingInput = false;
char inp_buffer[256];
size_t buffer_len;

const char scancode_to_char[] = {
  '?', '?', '1', '2', '3', '4', '5',
  '6', '7', '8', '9', '0', '-', '=',
  '\b', '?', 'q', 'w', 'e', 'r', 't',
  'y', 'u', 'i', 'o', 'p', '[', ']',
  '\n', '?', 'a', 's', 'd', 'f', 'g',
  'h', 'j', 'k', 'l', ';', '\\', '`',
  '?', '\\', 'z', 'x', 'c', 'v', 'b',
  'n', 'm', ',', '.', '/', '?', '?',
  '?', ' '
};


// char characterTable[] = {
//     0,    27,   '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',
//     '-',  '=',  '\b',    9,    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
//     'o',  'p',  '[',  ']',  0,    0,    'a',  's',  'd',  'f',  'g',  'h',
//     'j',  'k',  'l',  ';',  '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',
//     'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',  0,    ' ',  0,    0,
//     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
//     0,    0,    0,    0,    0,    0,    0,    0,    0x1B, 0,    0,    0,
//     0,    0,    0,    0,    0,    0,    0,    0x0E, 0x1C, 0,    0,    0,
//     0,    0,    0,    0,    0,    '/',  0,    0,    0,    0,    0,    0,
//     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
//     0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0,
//     0,    0,    0,    0,    0,    0,    0,    0x2C,
// };

// char shiftedCharacterTable[] = {
//     0,    27,   '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',
//     '_',  '+',  0,    9,    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
//     'O',  'P',  '{',  '}',  0,    0,    'A',  'S',  'D',  'F',  'G',  'H',
//     'J',  'K',  'L',  ':',  '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',
//     'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',  0,    ' ',  0,    0,
//     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
//     0,    0,    0,    0,    0,    0,    0,    0,    0x1B, 0,    0,    0,
//     0,    0,    0,    0,    0,    0,    0,    0x0E, 0x1C, 0,    0,    0,
//     0,    0,    0,    0,    0,    '?',  0,    0,    0,    0,    0,    0,
//     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
//     0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0,
//     0,    0,    0,    0,    0,    0,    0,    0x2C,
// };


void keyboard_init()
{
    memset(inp_buffer, '\0', 256);
    buffer_len = 0;
    irq_register_handler(1, keyboard_irq_handler);
}

// TODO: make a proper handler that handles deadlocks, and race conditions
void keyboard_irq_handler()
{
    uint8_t scancode = read_portb(0x60);
    if (scancode > 57) return;
    if (isRecordingInput == false) return;

    if (buffer_len >= 255) 
    {
        memset(inp_buffer, '\0', 256);
        buffer_len = 0;
    }

    char ch = scancode_to_char[scancode];
    if (ch == '\n') 
    {
        isRecordingInput = false;
        printf("\n");
        inp_buffer[buffer_len++] = '\0';
        return;
    }
    if (ch == '\b')
    {
        if (buffer_len == 0) return;
        inp_buffer[--buffer_len] = '\0';
        printf("\b");
        return;
    }
    printf("%c", ch);
    inp_buffer[buffer_len++] = ch;
}

void keyboard_get_line(char *buffer, size_t length)
{
    isRecordingInput = true;

    while (isRecordingInput) {};

 
    if (length > buffer_len) length = buffer_len;
    if (buffer != NULL) memcpy(buffer, inp_buffer, length);

    memset(inp_buffer, '\0', 256);
    buffer_len = 0;
}