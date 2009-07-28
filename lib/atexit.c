#include "stdlib.h"
static void (*funcs[ATEXIT_MAX])(void);
static size_t nfunc = 0;

int atexit(void (*f)(void))
{
    if (nfunc >= ATEXIT_MAX) return 1;
    funcs[nfunc++] = f;
    return 0;
}

/* Called from exit() defined in stdlib.c */
void _atexit_run()
{
    while (nfunc > 0)
    {
        (*funcs[--nfunc])();
        funcs[nfunc] = (void(*)(void))0;
    }
}
