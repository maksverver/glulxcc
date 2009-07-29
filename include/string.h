#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

#include "stdlib.h"

/* Implemented: */
#define memcpy(dest, src, n) memmove(dest, src, n)
extern void *memmove(void *dest, const void *src, size_t n);
extern void *memchr(const void *s, char c, size_t n);
extern int memcmp(const void *s1, const void *s2, size_t n);
extern void *memset(void *s, int c, size_t n);

extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern char *strchr(const char *s, int c);
extern char *strrchr(const char *s, int c);
extern char *strcat(char *dest, const char *src);
extern char *strncat(char *dest, const char *src, size_t n);
extern size_t strlen(const char *s);
extern int strcmp(const char *s1, const char *s2);
extern int strncmp(const char *s1, const char *s2, size_t n);

/* the following functions have low priority: */
/*
extern int strcoll(const char *, const char *);
extern size_t strspn(const char *s, const char *accept);
extern size_t strcspn(const char *s, const char *reject);
extern char *strpbrk(const char *s, const char *accept);
extern char *strstr(const char *haystack, const char *needle);
extern char *strtok(char *str, const char *delim);
extern size_t strxfrm(char *dest, const char *src, size_t n);
*/

#endif /* ndef STRING_H_INCLUDED */
