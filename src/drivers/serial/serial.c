#include "serial.h"
#include "cpu/io.h"
#include <stdbool.h>
#include "utils/sink.h"

static bool isInitialied = false;
static uint16_t current_com = 0;

static void serial_sink_write(char ch)
{
    if (!isInitialied) return;
    serial_write(ch);
}

void serial_configure_baud_rate(uint16_t com, uint16_t divisor)
{
    outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
    outb(SERIAL_DATA_PORT(com), (uint8_t)(divisor >> 8) & 0xff);
    outb(SERIAL_DATA_PORT(com), (uint8_t)(divisor) & 0xff);
}

int serial_init(uint16_t com)
{
    if (isInitialied) return -1;
    serial_configure_baud_rate(com, 3);
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0xc7);
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x16);
    outb(SERIAL_DATA_PORT(com), 0xae);

    if (inb(SERIAL_DATA_PORT(com)) != 0xae)
    {
        return -1;
    }

    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x7);
    isInitialied = true;
    current_com = com;
    return sink_register(serial_sink_write);
}

int is_transmit_empty(uint16_t com) {
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

void serial_write(char ch)
{
    if (!isInitialied) return;
    while (is_transmit_empty(current_com)==0);
    outb(SERIAL_DATA_PORT(current_com), ch);   
}


