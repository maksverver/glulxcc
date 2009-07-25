#include "stdlib.h"
#include "string.h"

/* NOTE: all of these can be implemented more efficiently with specialized
         Glulx opcodes (linearsearch, memset, memcpy) */

char *strcat(char *dest, const char *src)
{
    strcpy(dest + strlen(dest), src);
    return dest;
}

char *strchr(const char *s, int c)
{
    while (*s != c) if (!*++s) return NULL;
    return (char*)s;
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 == *s2 && *s1) ++s1, ++s2;
    return *s1 - *s2;
}

char *strcpy(char *dest, const char *src)
{
    char *res = dest;
    while (*src) *dest++ = *src++;
    return res;
}

size_t strlen(const char *s)
{
    size_t len = 0;
    while (*s++) ++len;
    return len;
}
