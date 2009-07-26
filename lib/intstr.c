/* Functions for converting between integers and strings. 
   NOT PART OF ANY STANDARD!

   These functions return a pointer to a static buffer which is invalidated
   whenever another libc function is called. */


/* Note: printf.c abuses the fact that there is some free room in front of the
         buffer returned to insert size prefixes (and itoa does this too to 
         add a minus-sign) so this should not be made too small: */
static char utox_buf[40];

/* Note: these are defined as macros because Glulx doesn't have proper support
         for unsigned division; inlining this allows the compiler to implement
         the octal/hexadecimal conversion routines with shifts instead. */
#define utox(name, digits, base)        \
                                        \
const char *name(unsigned u)            \
{                                       \
    char *p = &utox_buf[33];            \
    do {                                \
        *--p = digits[u%base];          \
        u /= base;                      \
    } while (u > 0);                    \
    return p;                           \
}

utox(otostr, "01234567",          8)
utox(utostr, "0123456789",       10)
utox(xtostr, "0123456789abcdef", 16)
utox(Xtostr, "0123456789ABCDEF", 16)

#undef utox

const char *itostr(int i)
{
    const char *p;
    if (i >= 0)
    {
        p = utostr((unsigned)i);
    }
    else
    if (i == 0x80000000)
    {
        return "-2147483648";
    }
    else
    {
        p = utostr((unsigned)-i);
        *(char*)--p = '-';
    }
    return p;
}
