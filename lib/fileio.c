#include "stdio.h"
#include "limits.h"
#include "glk.h"

FILE *stdin, *stdout, *stderr;

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size *= nmemb;
    glk_put_buffer_stream(stream, (char*)ptr, size);
    return size;
}

int fputs(const char *s, FILE *stream)
{
    glk_put_buffer_stream(stream, (char*)s, strlen(s));
    return 1;
}

int puts(const char *s)
{
    glk_put_buffer_stream(stdout, (char*)s, strlen(s));
    glk_put_char_stream(stdout, '\n');
    return 1;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t res = 0;
    while (nmemb > 0)
    {
        if (glk_get_buffer_stream(stream, ptr, size) != size) break;
        ptr = (char*)ptr + size;
        --nmemb;
    }
    return res;
}

char *fgets(char *s, int size, FILE *stream)
{
    return (size == 0 || glk_get_line_stream(stream, s, size) <= 0) ? NULL : s;
}
