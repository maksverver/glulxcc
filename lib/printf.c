#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "intstr.h"

/* Defined in instr.c */
extern const char *itostr(int i);       /* integer to decimal string */
extern const char *otostr(unsigned u);  /* unsigned to octal string */
extern const char *utostr(unsigned u);  /* unsigned to decimal string */
extern const char *xtostr(unsigned u);  /* unsigned to lowercase hex. string */
extern const char *Xtostr(unsigned u);  /* unsigned to uppercase hex. string */


static int write(const char *s)
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

/* Writes `len' padding characters and returns the number of characters written;
   note that `len' may be negative, nothing is written and 0 is returned. */
static int write_padding(int len, int ch)
{
    int i;
    for (i = 0; i < len; ++i) putchar(ch);
    return i;
}

static int writepad(const char *s, int padto, int padch)
{
    if (padto == 0)
        return write(s);
    else
    if (padto > 0)
        return write_padding(padto - strlen(s), padch) + write(s);
    else  /* padto < 0 */
        return write(s) + write_padding(-padto - strlen(s), padch);
}

int putchar(int c)
{
    glk_put_char(c);
    return c;
}

int puts(const char *s)
{
    int n = write(s);
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
            const char *start = fmt;
            int alt = 0, align = 0;
            int padto = 0;
            char padch = ' ';

            ++fmt;  /* skip % */

            /* Parse flags */
            for (;;)
            {
                switch (*fmt)
                {
                case '0': padch = '0'; align = 0; break;
                case '#': alt = 1; break;
                case '-': padch = ' '; align = 1; break;
                case 'l': /* long; ignored */ break;
                default: goto end_of_flags;
                }
                ++fmt;
            }
        end_of_flags:

            /* Parse padding width */
            for ( ; *fmt >= '0' && *fmt <= '9'; ++fmt)
                padto = 10*padto + (*fmt - '0');
            if (align) padto = -padto;

            /* Parse length modifiers (all ignored) */
            for (;;)
            {
                switch (*fmt)
                {
                case 'h':
                case 'l':
                case 'L':
                case 'q':
                case 'j':
                case 'z':
                case 't':
                    break;
                default: goto end_of_length;
                }
                ++fmt;
            }
        end_of_length:

            switch (*fmt)
            {
            case '%':  /* literal percentage */
                putchar('%');
                ++written;
                break;

            case 'd':  /* signed decimal */
            case 'i':
                written += writepad(itostr(va_arg(ap, int)), padto, padch);
                break;

            case 'u':  /* unsigned decimal */
                written += writepad(utostr(va_arg(ap, unsigned)), padto, padch);
                break;

            case 'o':  /* unsigned octal */
                {
                    unsigned u = va_arg(ap, unsigned);
                    if (alt && u != 0)
                    {
                        putchar('0');
                        ++written;
                    }
                    written += writepad(otostr(u), padto, padch);
                } break;

            case 'p':  /* unsigned pointer */
                alt = 1;
                /* falls through */
            case 'x':  /* unsigned lowercase hexadecimal */
                if (alt) written += write("0x");
                written += writepad(xtostr(va_arg(ap, unsigned)), padto, padch);
                break;

            case 'X':  /* unsigned uppercase hexadecimal */
                if (alt) written += write("0x");
                written += writepad(Xtostr(va_arg(ap, unsigned)), padto, padch);
                break;

            case 'c':  /* single character */
                putchar(va_arg(ap, int));
                break;

            case 's':  /* zero-terminated string */
                written += writepad(va_arg(ap, const char*), padto, padch);
                break;

            default:  /* unrecognized option */
                for ( ; start <= fmt; ++start) putchar(*start);
                break;
            }
        }
    }
    va_end(ap);
    return written;
}
