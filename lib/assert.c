#include "stdio.h"
#include "stdlib.h"

void __assert_failed(const char *file, int line, const char *expr)
{
    /* FIXME: fprintf(stderr, ...) instead (once fprintf is implemented)  */
    printf("Assertion `%s' failed on line %d of %s.\n", expr, line, file);
    abort();
}
