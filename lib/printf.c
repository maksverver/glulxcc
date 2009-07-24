#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"

static const char *utoa(unsigned u)
{
    static char buf[12] = { 0 };
    char *p = &buf[11];
    do {
        *--p = '0' + u%10;
        u /= 10;
    } while (u > 0);
    return p;
}

static const char *itoa(int i)
{
    const char *p;
    if (i >= 0)
    {
        p = utoa((unsigned)i);
    }
    else
    {
        p = utoa((unsigned)-i);
        *(char*)--p = '-';
    }
    return p;
}

static int _write(const char *s)
{
    int written = 0;
    if (s != NULL)
    {
        while (*s)
        {
            putchar(*s++);
            ++written;
        }
    }
    return written;
}

int putchar(int c)
{
    glk_put_char(c);
    return c;
}

int puts(const char *s)
{
    int n = _write(s);
    putchar('\n');
    return n + 1;
}

int printf(const char *fmt, ...)
{
    int written = 0;
    va_list ap;
    va_start(ap, fmt);
    for ( ; *fmt; ++fmt)
    {
        if (*fmt != '%')
        {
            putchar(*fmt);
            ++written;
        }
        else
        {
            switch (*++fmt)
            {
            default:
                putchar('%');
                ++written;
                /* falls through */
            case '%':
                putchar(*fmt);
                ++written;
                break;

            case 'd':
            case 'i':
                written += _write(itoa(va_arg(ap, int)));
                break;

            case 'u':
                written += _write(utoa(va_arg(ap, unsigned)));
                break;

            case 'c':
                putchar(va_arg(ap, int));
                break;

            case 's':
                written += _write(va_arg(ap, const char*));
                break;

            /* o, u, x, p: unsigned octal, decimal, hexadecimal, pointer */
            }
        }
    }
    va_end(ap);
    return written;
}
