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
#define ATEXIT_MAX 32
int atexit(void (*function)(void));

/* mssing: malloc, calloc, realloc, free, */

typedef struct {
    int quot;
    int rem;
} div_t;

div_t div(int numerator, int denominator);

typedef struct {
    unsigned quot;
    unsigned rem;
} udiv_t;

/* NB. NOT a standard function; do not use unless performance is critical! */
udiv_t udiv(unsigned numerator, unsigned denominator);

#endif /* ndef STDLIB_H_INCLUDED */
