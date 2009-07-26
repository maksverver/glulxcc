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

#define stdin  ((FILE*)glk_stream_get_current())
#define stdout ((FILE*)glk_stream_get_current())
#define stderr ((FILE*)glk_stream_get_current())

/* putc/puts */
#define fputc(c, stream)    glk_put_char_stream(stream, c)
int fputs(const char *s, FILE *stream);
int puts(const char *s);
#define putc(c, stream)     glk_put_char_stream(stream, c)
#define putchar(c)          glk_put_char(c)

/* getc/gets */
extern int fgetc(FILE *stream);
extern char *fgets(char *s, int size, FILE *stream);
extern char *gets(char *s);
#define getc(stream)        fgetc(stream)
#define getchar()           getc(stream)

/* printf (TODO) */
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
int vprintf(const char *format, va_list ap);
int vfprintf(FILE *stream, const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

/* scanf (TODO) */
int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *str, const char *format, ...);
int vscanf(const char *format, va_list ap);
int vsscanf(const char *str, const char *format, va_list ap);
int vfscanf(FILE *stream, const char *format, va_list ap);

/* File I/O (all TODO) */
extern FILE *fopen(const char *path, const char *mode);
extern int fclose(FILE *fp);
extern int fseek(FILE *stream, long offset, int whence);
extern long ftell(FILE *stream);
extern void rewind(FILE *stream);
extern int fgetpos(FILE *stream, fpos_t *pos);
extern int fsetpos(FILE *stream, fpos_t *pos);
extern int fflush(FILE *stream);
extern void clearerr(FILE *stream);
extern int feof(FILE *stream);
extern int ferror(FILE *stream);
extern int fileno(FILE *stream);

#endif /* ndef STDIO_H_INCLUDED */
