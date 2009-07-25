#ifndef STDLIB_H_INCLUDED
#define STDLIB_H_INCLUDED

typedef unsigned size_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

#define atol atoi
extern int atoi(const char *nptr);
#define RAND_MAX 2147483647
extern int rand(void);
extern void srand(unsigned seed);
extern void abort(void);
extern void exit(int status);
extern char *getenv(const char *name);

/* mssing: malloc, calloc, realloc, free, atexit */

#endif /* ndef STDLIB_H_INCLUDED */
