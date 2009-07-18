#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

#include "stdlib.h"

char *strcat(char *dest, const char *src);
char *strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
size_t strlen(const char *s);

#endif /* ndef STRING_H_INCLUDED */
