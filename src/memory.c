#pragma once
#include "memory.h"

void *memcpy(void *destPtr, const void *srcPtr, size_t size)
{
    uint8_t *dest = (uint8_t *)destPtr;
    uint8_t *src = (uint8_t *)destPtr;
    for (size_t i = 0; i < size; i++)
    {
        dest[i] = src[i];
    }
    return destPtr;
}

void *memmove(void *destPtr, const void *srcPtr, size_t size)
{
    uint8_t *dest = (uint8_t *)destPtr;
    uint8_t *src = (uint8_t *)destPtr;

    if (srcPtr < destPtr)
    {
        for (size_t i = 0; i < size; i++)
        {
            dest[i] = src[i];
        }
    }
    else
    {
        for (size_t i = size; i != 0; i--)
        {
            dest[i - 1] = src[i - 1];
        }
    }
    return destPtr;
}

void *memset(void *destPtr, int value, size_t size)
{
    uint8_t *dest = (uint8_t *)destPtr;

    for (size_t i = 0; i < size; i++)
    {
        dest[i] = (uint8_t)value;
    }
    return destPtr;
}

void *memsetw(void *destPtr, int value, size_t size)
{
    uint16_t *dest = (uint16_t *)destPtr;

    for (size_t i = 0; i < size; i++)
    {
        dest[i] = (uint16_t)value;
    }
    return destPtr;
}

int memcmp(const void *ptr1, const void *ptr2, size_t size)
{
    uint8_t *mem1 = (uint8_t*)ptr1;
    uint8_t *mem2 = (uint8_t*)ptr2;
    
    for (size_t i =0; i < size; i++)
    {
        if (mem1[i] < mem2[i]) return -1;
        else if (mem1[i] > mem2[i]) return 1;
    }
    return 0;
}