#include "memory_utils.h"

void *memcpy(void *destPtr, const void *srcPtr, size_t size)
{
    uint8_t *dest = (uint8_t *)destPtr;
    const uint8_t *src = (const uint8_t *)srcPtr;
    for (size_t i = 0; i < size; i++)
    {
        dest[i] = src[i];
    }
    return destPtr;
}

void *memmove(void *destPtr, const void *srcPtr, size_t size)
{
    uint8_t *dest = (uint8_t *)destPtr;
    const uint8_t *src = (const uint8_t *)srcPtr;

    if (dest > src)
    {
        for (size_t i = size; i != 0; i--)
            dest[i - 1] = src[i - 1];
    }
    else
    {
        for (size_t i = 0; i < size; i++)
            dest[i] = src[i];
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

void *memsetw(void *destPtr, int value, size_t count)
{
    uint16_t *dest = (uint16_t *)destPtr;

    for (size_t i = 0; i < count; i++)
    {
        dest[i] = (uint16_t)value;
    }
    return destPtr;
}

int memcmp(const void *ptr1, const void *ptr2, size_t size)
{
    const uint8_t *mem1 = (const uint8_t *)ptr1;
    const uint8_t *mem2 = (const uint8_t *)ptr2;

    for (size_t i = 0; i < size; i++)
    {
        if (mem1[i] != mem2[i])
            return (int)mem1[i] - (int)mem2[i];
    }

    return 0;
}

size_t strlen(const char *s)
{
    size_t i = 0;
    while (s[i] != '\0')
        i++;
    return i;
}

char *strcpy(char *dest, const char *src)
{
    size_t i = 0;

    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';
    return dest;
}

char *strsplit(char *str, char delim)
{
    if (str == NULL) return NULL;
    if (delim == '\0') return NULL;

    // Skip leading delim
    while (*str == delim) str++;
    
    size_t i = 0;
    while (str[i] != '\0' && str[i] != delim)
        i++;
    
    if (str[i] == '\0') return NULL;
    str[i] = '\0';
    return &str[i+1];
}

int strcmp(const char *s1, const char *s2)
{

    for (size_t i = 0;; i++)
    {
        if (s1[i] != s2[i])
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        if (s1[i] == '\0')
            return 0;
    }
}