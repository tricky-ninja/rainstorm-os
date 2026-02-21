#pragma once
#include <stdint.h>
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t size);
void *memset(void *dest, int value, size_t size);
void *memsetw(void *dest, int value, size_t count);
void *memmove(void *dest, const void *src, size_t size);
int memcmp(const void *s1, const void *s2, size_t size);
int strcmp(const char *s1, const char *s2);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);

/**
 * strsplit - Split a string in-place by a delimiter.
 *
 * Replaces the first occurrence of `delim` with '\0'.
 *
 * @param str    Writable null-terminated string (may be NULL).
 * @param delim  Delimiter character (must not be '\0').
 *
 * @return Pointer to the next part of the string,
 *         or NULL if no delimiter is found or input is NULL.
 *
 * Note: The input string is modified.
 */
char *strsplit(char *str, char delim);