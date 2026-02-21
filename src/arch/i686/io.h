#pragma once

#include <stdint.h>

uint8_t read_portb(uint16_t port);
void write_portb(uint16_t port, uint8_t data);
void io_wait();
void io_enableInterrupts();
void io_disableInterrupts();