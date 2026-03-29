#pragma once

#include <stdint.h>

/* https://wiki.osdev.org/Serial_Ports */
// Base I/O port address for COM1
#define SERIAL_COM1_BASE 0x3F8

/**
 * UART Register Layout (relative to base address)
 *
 *  Offset | Register
 *  -------|--------------------------
 *   +0    | Data Register (THR/RBR)
 *   +2    | FIFO Control Register
 *   +3    | Line Control Register
 *   +4    | Modem Control Register
 *   +5    | Line Status Register
 *   +6    | Modem Status Register
 *   +7    | Scratch Register
 */
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)
#define SERIAL_MODEM_STATUS_PORT(base)  (base + 6)
#define SERIAL_SCRATCH_PORT(base)       (base + 7)

/* The I/O port commands */

/* SERIAL_LINE_ENABLE_DLAB:
 * Tells the serial port to expect first the highest 8 bits on the data port,
 * then the lowest 8 bits will follow
 */
#define SERIAL_LINE_ENABLE_DLAB 0x80

/** serial_configure_baud_rate:
*  Sets the speed of the data being sent. The default speed of a serial
*  port is 115200 bits/s. The argument is a divisor of that number, hence
*  the resulting speed becomes (115200 / divisor) bits/s.
*
*  @param com      The COM port to configure
*  @param divisor  The divisor
*/
void serial_configure_baud_rate(uint16_t com, uint16_t divisor);

/**
 *  Initializes a serial port.
 *
 *  Performs basic UART configuration including:
 *      - Baud rate setup
 *      - Data bits configuration
 *      - FIFO setup
 *      - Modem control configuration
 *
 *  @param com Base I/O port address of COM port
 *  @return 0 on success, non-zero on failure
 */
int serial_init(uint16_t com);

void serial_write(char ch);