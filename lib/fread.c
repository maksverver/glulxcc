#include "stdio.h"
#include "limits.h"
#include "glk.h"

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

char *gets(char *s)
{
    strid_t stream = glk_stream_get_current();
    int nread = glk_get_line_stream(stream, s, INT_MAX);
    if (nread <= 0) return NULL;
    --nread;
    if (s[nread] == '\n') s[nread] = '\0';
    return s;
}
