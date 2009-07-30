#include "stdio.h"
#include "limits.h"
#include "glk.h"

FILE *stdin, *stdout, *stderr;

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    glk_put_buffer_stream(stream, (char*)ptr, size*nmemb);
    return nmemb;
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
    size_t nread;
    for (nread = 0; nread < nmemb; ++nread)
    {
        if (glk_get_buffer_stream(stream, ptr, size) != size) break;
        ptr = (char*)ptr + size;
    }
    return nread;
}

char *fgets(char *s, int size, FILE *stream)
{
    return (size == 0 || glk_get_line_stream(stream, s, size) <= 0) ? NULL : s;
}

FILE *tmpfile(void)
{
    frefid_t fref;
    strid_t str;
    fref = glk_fileref_create_temp(fileusage_Data | fileusage_BinaryMode, 0);
    if (fref == NULL) return NULL;

    /* Open in write-only mode to force file creation */
    str = glk_stream_open_file(fref, filemode_Write, 0);
    if (str == NULL) goto done;
    glk_stream_close(str, NULL);

    /* Re-open in read/write mode as required by ANSI-C */
    str = glk_stream_open_file(fref, filemode_ReadWrite, 0);

done:
    glk_fileref_destroy(fref);
    return str;
}

static frefid_t temp_fref_by_name(glui32 usage, const char *name)
{
    static char buf[100];
    if (snprintf(buf, sizeof(buf), "%c%s", 0xE0, name) >= sizeof(buf))
        return NULL;
    return glk_fileref_create_by_name(usage, buf, 0);
}

int remove(const char *pathname)
{
    frefid_t fref = temp_fref_by_name(fileusage_Data, pathname);
    if (glk_fileref_does_file_exist(fref))
        glk_fileref_delete_file(fref);
    return 0;
}

FILE *fopen(const char *path, const char *mode)
{
    glui32 usage = fileusage_Data | fileusage_BinaryMode, fmode = 0;
    frefid_t fref;
    strid_t str;

    switch (*mode++)
    {
    case 'r': fmode = filemode_Read; break;
    case 'w': fmode = filemode_Write; break;
    case 'a': fmode = filemode_WriteAppend; break;
    default: return NULL;
    }

    if (*mode == 'b') ++mode;

    if (*mode == '+')
    {
        if (fmode == filemode_Read || fmode == filemode_Write)
            fmode = filemode_ReadWrite;
        else
            return NULL;
        ++mode;
    }

    for ( ; *mode; ++mode)
    {
        if (*mode == 't')
        {
            usage = fileusage_Data | fileusage_TextMode;
            break;
        }
    }

    fref = temp_fref_by_name(usage, path);
    if (fref == NULL) return NULL;
    str = glk_stream_open_file(fref, fmode, 0);
    glk_fileref_destroy(fref);
    return str;
}

int fclose(FILE *stream)
{
    glk_stream_close(stream, NULL);
    return 0;
}

int fseek(FILE *stream, long offset, int whence)
{
    glk_stream_set_position(stream, offset, whence);
    return glk_stream_get_position(stream);
}

long ftell(FILE *stream)
{
    long res = (long)glk_stream_get_position(stream);
    if (res < 0) res = -1;
    return res;
}

void rewind(FILE *stream)
{
    /* FIXME: clear error indicators as well (when they are implemented) */
    glk_stream_set_position(stream, 0, seekmode_Start);
}

int fgetpos(FILE *stream, fpos_t *pos)
{
    *pos = glk_stream_get_position(stream);
    return 0;
}

int fsetpos(FILE *stream, fpos_t *pos)
{
    glk_stream_set_position(stream, *pos, seekmode_Start);
    return 0;
}

int fflush(FILE *stream)
{
    (void)stream;  /* unused */
    return 0;
}
