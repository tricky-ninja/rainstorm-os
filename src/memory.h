#pragma once
#include <stdint.h>
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t size);
void *memset(void *dest, int value, size_t size);
void *memsetw(void *dest, int value, size_t count);
void *memmove(void *dest, const void *src, size_t size);
int memcmp(const void *s1, const void *s2, size_t size);