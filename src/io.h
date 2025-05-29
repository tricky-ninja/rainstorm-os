#pragma once

#include <stdint.h>

uint8_t x86_64_read_port_byte(uint16_t port);
void x86_64_write_port_byte(uint16_t port, uint8_t data);