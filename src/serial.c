#include "serial.h"
#include "io.h"

void serial_configure_baud_rate(uint16_t com, uint16_t divisor)
{
    i686_write_port_byte(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
    i686_write_port_byte(SERIAL_DATA_PORT(com), (uint8_t)(divisor >> 8) & 0xff);
    i686_write_port_byte(SERIAL_DATA_PORT(com), (uint8_t)(divisor) & 0xff);
}

int serial_init(uint16_t com)
{
    serial_configure_baud_rate(com, 3);
    i686_write_port_byte(SERIAL_LINE_COMMAND_PORT(com), 0x03);
    i686_write_port_byte(SERIAL_FIFO_COMMAND_PORT(com), 0xc7);
    i686_write_port_byte(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
    i686_write_port_byte(SERIAL_MODEM_COMMAND_PORT(com), 0x16);
    i686_write_port_byte(SERIAL_DATA_PORT(com), 0xae);

    if (i686_read_port_byte(SERIAL_DATA_PORT(com)) != 0xae)
    {
        return 1;
    }

    i686_write_port_byte(SERIAL_MODEM_COMMAND_PORT(com), 0x7);
    return 0;

}

int is_transmit_empty(uint16_t com) {
    return i686_read_port_byte(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

void serial_write(uint16_t com, char ch)
{
    while (is_transmit_empty(com)==0);
    i686_write_port_byte(SERIAL_DATA_PORT(com), ch);   
}
