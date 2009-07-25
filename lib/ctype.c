#include "ctype.h"

int isalnum(int c)
{
    return isdigit(c) || isalpha(c);
}

int isalpha(int c)
{
    return isupper(c) || islower(c);
}

int isascii(int c)
{
    return (unsigned)c <= 127;
}

int isdigit(int c)
{
    return c >= '0' && c <= '9';
}

int isupper(int c)
{
    /* FIXME: implement this as tolower(c) != c instead? */
    return c >= 'A' && c <= 'Z';
}

int islower(int c)
{
    /* FIXME: implement this as toupper(c) != c instead? */
    return c >= 'a' && c <= 'z';
}

int isblank(int c)
{
    return c == 32 || c == 9;  /* space, tab */
}

int iscntrl(int c)
{
    return (unsigned)c <= 31 || c == 127;
}

int isgraph(int c)
{
    return c >= 33 && c <= 126;
}

int isprint(int c)
{
    return c >= 32 && c <= 126;
}

int ispunct(int c)
{
    return isgraph(c) && !isalnum(c);
}

int isspace(int c)
{
    return c == 32 || (c >= 9 && c <= 13);  /* space, tab, lf, vt, ff, cr */
}

int isxdigit(int c)
{
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

int tolower(int c)
{
    /* FIXME: implement this with Glk instead? */
    return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

int toupper(int c)
{
    /* FIXME: implement this with Glk instead? */
    return (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c;
}
