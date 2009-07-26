#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "glulx.h"

/* Defined in instr.c */
extern const char *itostr(int i);       /* integer to decimal string */
extern const char *otostr(unsigned u);  /* unsigned to octal string */
extern const char *utostr(unsigned u);  /* unsigned to decimal string */
extern const char *xtostr(unsigned u);  /* unsigned to lowercase hex. string */
extern const char *Xtostr(unsigned u);  /* unsigned to uppercase hex. string */

/* Writes `len' padding characters and returns the number of characters written;
   note that `len' may be negative, nothing is written and 0 is returned. */
static int write_padding(FILE *stream, int len, int ch)
{
    int i;
    for (i = 0; i < len; ++i) fputc(ch, stream);
    return i;
}

/* Writes formatted data to a stream and returns the number of bytes that
   would be written. (No checks for failure occur.) */
int vfprintf(FILE *fp, const char *fmt, va_list ap)
{
    int written = 0;
    for ( ; *fmt; ++fmt)
    {
        if (*fmt != '%')
        {
            putc(*fmt, fp);
            ++written;
        }
        else
        {
            const char *start = fmt;
            int alt = 0, align = 0;
            int padto = 0;
            char padch = ' ';
            const char *str_out = NULL;  /* string to write */
            int out_len;

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
                /* write it here directly instead of doing str_out="%" to
                   avoid padding: */
                putc('%', fp);
                ++written;
                break;

            case 'd':  /* signed decimal */
            case 'i':
                str_out = itostr(va_arg(ap, int));
                break;

            case 'u':  /* unsigned decimal */
                str_out = utostr(va_arg(ap, unsigned));
                break;

            case 'o':  /* unsigned octal */
                str_out = otostr(va_arg(ap, unsigned));
                if (alt && *str_out != 0) *(char*)--str_out = '0';
                break;

            case 'p':  /* unsigned pointer */
                alt = 1;
                /* falls through */
            case 'x':  /* unsigned lowercase hexadecimal */
            case 'X':  /* unsigned uppercase hexadecimal */
                str_out = (*fmt == 'X') ? Xtostr(va_arg(ap, unsigned))
                                        : xtostr(va_arg(ap, unsigned));
                if (alt)
                {
                    *(char*)--str_out = 'x';
                    *(char*)--str_out = '0';
                }
                break;

            case 'c':  /* single character */
                {
                    static char buf[2] = { '\0', '\0' };
                    buf[0] = va_arg(ap, int);
                    str_out = buf;
                } break;

            case 's':  /* zero-terminated string */
                str_out = va_arg(ap, const char*);
                break;

            default:  /* unrecognized option */
                for ( ; start <= fmt; ++start)
                {
                    putc(*start, fp);
                    ++written;
                }
                break;
            }

            if (str_out == NULL) continue;

            /* Write string to output: */
            out_len = (int)strlen(str_out);
            written += out_len;
            if (padto == 0)  /* no padding -- common case */
            {
                fputs(str_out, fp);
            }
            else
            if (padto > 0)  /* right-align */
            {
                written += write_padding(fp, padto - out_len, padch);
                fputs(str_out, fp);
            }
            else    /* padto < 0; left-align */
            {
                fputs(str_out, fp);
                written += write_padding(fp, -padto - out_len, padch);
            }
        }
    }
    return written;
}


int printf(const char *format, ...)
{
    int res;
    va_list ap;
    va_start(ap, format);
    res = vfprintf(stdout, format, ap);
    va_end(ap);
    return res;
}

int fprintf(FILE *stream, const char *format, ...)
{
    int res;
    va_list ap;
    va_start(ap, format);
    res = vfprintf(stdout, format, ap);
    va_end(ap);
    return res;
}

int sprintf(char *str, const char *format, ...)
{
    int res;
    va_list ap;
    va_start(ap, format);
    res = vsprintf(str, format, ap);
    va_end(ap);
    return res;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
    int res;
    va_list ap;
    va_start(ap, format);
    res = vsnprintf(str, size, format, ap);
    va_end(ap);
    return res;
}

int vprintf(const char *format, va_list ap)
{
    return vfprintf(stdout, format, ap);
}

int vsprintf(char *str, const char *format, va_list ap)
{
    return vsnprintf(str, glulx_getmemsize() - (unsigned)str, format, ap);
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    int res;
    FILE *fp;
    fp = glk_stream_open_memory(str, size, filemode_Write, 0);
    if (fp == NULL) return -1;
    res = vfprintf(fp, format, ap);
    glk_stream_close(fp, NULL);
    if (res < size)
        str[res] = '\0';
    else
    if (size > 0)
        str[size - 1] = '\0';
    return res;
}
