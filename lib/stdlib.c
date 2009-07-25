#include "stdlib.h"
#include "ctype.h"
#include "glulx.h"

int atoi(const char *nptr)
{
    int res = 0, neg;
    while (isspace(*nptr)) ++nptr;
    neg = (*nptr == '-');
    if (*nptr == '+' || *nptr == '-') ++nptr;
    for ( ; *nptr >= '0' && *nptr <= '9'; ++nptr) res = 10*res + *nptr;
    return neg ? -res : res;
}

int rand()
{
    return glulx_random(0)&0x7fffffff;
}

void srand(unsigned seed)
{
    glulx_setrandom(seed);
}

void abort(void)
{
    glulx_debugtrap(0);
}

void exit(int status)
{
    (void)status;  /* ignored */
    glulx_quit();
}

char *getenv(const char *name)
{
    (void)name;  /* ignored */
    return NULL;
}
