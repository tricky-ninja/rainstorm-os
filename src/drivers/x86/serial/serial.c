    /**
     *  Serial Driver Implementation
     *
     *  Polling-based UART driver.
     *  Primarily used for debugging via QEMU (-serial stdio).
     *
     *  No interrupt-driven I/O yet.
     *  Author: Sreyas A (TrickyNinja)
     */

    #include "serial.h"
    #include "arch/i686/io.h"
    #include <stdbool.h>

    bool isInitialied = false;

    void serial_configure_baud_rate(uint16_t com, uint16_t divisor)
    {
        write_portb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);
        write_portb(SERIAL_DATA_PORT(com), (uint8_t)(divisor >> 8) & 0xff);
        write_portb(SERIAL_DATA_PORT(com), (uint8_t)(divisor) & 0xff);
    }

    int serial_init(uint16_t com)
    {
        serial_configure_baud_rate(com, 3);
        write_portb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
        write_portb(SERIAL_FIFO_COMMAND_PORT(com), 0xc7);
        write_portb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
        write_portb(SERIAL_MODEM_COMMAND_PORT(com), 0x16);
        write_portb(SERIAL_DATA_PORT(com), 0xae);

        if (read_portb(SERIAL_DATA_PORT(com)) != 0xae)
        {
            return 1;
        }

        write_portb(SERIAL_MODEM_COMMAND_PORT(com), 0x7);
        isInitialied = true;
        return 0;

    }

    int is_transmit_empty(uint16_t com) {
        return read_portb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
    }

    void serial_write(uint16_t com, char ch)
    {
        if (!isInitialied) return;
        while (is_transmit_empty(com)==0);
        write_portb(SERIAL_DATA_PORT(com), ch);   
    }
