#include "stdio.h"
#include "glk.h"

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
    strid_t stream = glk_stream_get_current();
    glk_put_buffer_stream(stream, (char*)s, strlen(s));
    glk_put_char_stream(stream, '\n');
    return 1;
}
