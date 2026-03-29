#include "memory.h"

void *memmove(void *dest, const void *src, size_t count)
{
    uint8_t *destPtr = (uint8_t *)dest;
    const uint8_t *srcPtr = (const uint8_t *)src;
    if (destPtr < srcPtr || destPtr >= srcPtr + count) {
        // no overlap or dest is before src, forward copy is safe
        for (size_t i = 0; i < count; i++)
            destPtr[i] = srcPtr[i];
    } else {
        // dest overlaps src from behind, copy backwards
        for (size_t i = count; i != 0; i--)
            destPtr[i - 1] = srcPtr[i - 1];
    }
    return dest;
}

void *memset(void *dest, int value, size_t count)
{
    uint8_t *destPtr = (uint8_t *)dest;

    for (size_t i = 0; i < count; i++)
    {
        destPtr[i] = (uint8_t)value;
    }
    return dest;
}

void *memsetw(void *dest, int value, size_t count)
{
    uint16_t *destPtr = (uint16_t *)dest;

    for (size_t i = 0; i < count; i++)
    {
        destPtr[i] = (uint16_t)value;
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t count)
{
    const uint8_t *mem1 = (const uint8_t *)s1;
    const uint8_t *mem2 = (const uint8_t *)s2;

    for (size_t i = 0; i < count; i++)
    {
        if (mem1[i] != mem2[i])
            return (int)mem1[i] - (int)mem2[i];
    }

    return 0;
}
