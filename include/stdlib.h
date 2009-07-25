#ifndef STDLIB_H_INCLUDED
#define STDLIB_H_INCLUDED

typedef unsigned size_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef atol
#define atol atoi
#endif

extern int atoi(const char *nptr);
extern int rand(void);
extern void srand(unsigned seed);
extern void abort(void);
extern void exit(int status);
extern char *getenv(const char *name);

/* mssing: malloc, calloc, realloc, free, atexit */

#endif /* ndef STDLIB_H_INCLUDED */
