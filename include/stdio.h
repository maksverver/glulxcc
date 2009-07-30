#ifndef STDIO_H_INCLUDED
#define STDIO_H_INCLUDED

#include "stdlib.h"
#include "stdarg.h"
#include "glk.h"

#define EOF            -1   /* end-of-file character */
#define SEEK_SET        0   /* equal to GLK's seekmode_Start */
#define SEEK_CUR        1   /* equal to GLK's seekmode_Current */
#define SEEK_END        2   /* equal to GLK's seekmode_End */

typedef struct glk_stream_struct FILE;
typedef unsigned int fpos_t;

extern FILE *stdin, *stdout, *stderr;

/* fwrite/fputs/puts/putchar/fputc/putc/putchar */
extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
extern int fputs(const char *s, FILE *stream);
extern int puts(const char *s);
extern int fputc(int c, FILE *stream);
extern int putchar(int c);
#define fputc(c, stream)    glk_put_char_stream(stream, c)
#define putc(c, stream)     glk_put_char_stream(stream, c)
#define putchar(c)          glk_put_char(c)

/* fread/fgets/gets/getchar/fgetc/getc/getchar */
extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern char *fgets(char *s, int size, FILE *stream);
/* extern char *gets(char *s); */
extern int fgetc(FILE *stream);
/* extern int getchar(void); */
#define getc(stream)        fgetc(stream)

/* printf */
extern int printf(const char *format, ...);
extern int fprintf(FILE *stream, const char *format, ...);
extern int sprintf(char *str, const char *format, ...);
extern int snprintf(char *str, size_t size, const char *format, ...);
extern int vprintf(const char *format, va_list ap);
extern int vfprintf(FILE *stream, const char *format, va_list ap);
extern int vsprintf(char *str, const char *format, va_list ap);
extern int vsnprintf(char *str, size_t size, const char *format, va_list ap);

/* scanf */
/*
extern int scanf(const char *format, ...);
extern int fscanf(FILE *stream, const char *format, ...);
extern int sscanf(const char *str, const char *format, ...);
extern int vscanf(const char *format, va_list ap);
extern int vsscanf(const char *str, const char *format, va_list ap);
extern int vfscanf(FILE *stream, const char *format, va_list ap);
*/

/* File I/O */
extern FILE *fopen(const char *path, const char *mode);
extern int fclose(FILE *stream);
extern int fseek(FILE *stream, long offset, int whence);
extern long ftell(FILE *stream);
extern void rewind(FILE *stream);
extern int fgetpos(FILE *stream, fpos_t *pos);
extern int fsetpos(FILE *stream, fpos_t *pos);
extern int fflush(FILE *stream);
/*
extern void clearerr(FILE *stream);
extern int feof(FILE *stream);
extern int ferror(FILE *stream);
extern int fileno(FILE *stream);
*/

extern FILE *tmpfile(void);
extern int remove(const char *pathname);

#endif /* ndef STDIO_H_INCLUDED */
