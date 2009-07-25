#include "stdlib.h"
#include "ctype.h"

int atoi(const char *nptr)
{
    int res = 0, neg;
    while (isspace(*nptr)) ++nptr;
    neg = (*nptr == '-');
    if (*nptr == '+' || *nptr == '-') ++nptr;
    for ( ; *nptr >= '0' && *nptr <= '9'; ++nptr) res = 10*res + *nptr;
    return neg ? -res : res;
}

char *getenv(const char *name)
{
    return NULL;
}
