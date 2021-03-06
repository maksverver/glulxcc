/* Glulx back-end by Maks Verver <maksverver@geocities.com> July 2009 */

#include <string.h>

#ifndef PREFIX
#error "PREFIX not set"
#else
#define LIBDIR     PREFIX "/lib/glulxcc/"
#define LIBEXECDIR LIBDIR
#define INCDIR     PREFIX "/include/glulxcc/"
#endif

char *suffixes[] = {
    ".c",       /* C source file */
    ".i",       /* preprocessed source file */
    ".ula",     /* Glulx assembly file */
    ".ulo",     /* Glulx object file */
    ".ulx",     /* Glulx excutable module */
    0 };

char inputs[256] = "";

/* Commands to preprocess, compile, assemble and link.
   Arguments of the form $x expand into a list of arguments:
    $1 == user options
    $2 == input file(s)
    $3 == output file
   The expanded lists can be empty (i.e. if there are no options given). */
char *cpp[] = { LIBEXECDIR "cpp", "-Dglulx", "-D__glulx__", "$1", "$2", "$3", 0 };
char *com[] = { LIBEXECDIR "rcc", "-target=glulx", "$1", "$2", "$3", 0 };
char *as[]  = { LIBEXECDIR "glulxas", "$1", "$2", "$3", 0 };
char *ld[]  = { LIBEXECDIR "glulxld", "-L" LIBDIR, "-o", "$3", "$1", "$2", 0 };
char *include[] = { "-I" INCDIR, 0 };

/* Parse options */
int option(char *arg)
{
    if (strncmp(arg, "-lccdir=", 8) == 0)
    {
        extern char *concat(char *, char *);
        cpp[0] = concat(&arg[8], "/cpp");
        com[0] = concat(&arg[8], "/rcc");
        as[0]  = concat(&arg[8], "/glulxas");
        ld[0]  = concat(&arg[8], "/glulxld");
        ld[1]  = concat("-L", &arg[8]);
        return 1;
    }

    if (strcmp(arg, "-p") == 0 || strcmp(arg, "-pg") == 0)
    {
        /* emit profiling code; see prof(1) and grpof(1) */
        return 1;
    }

    if (strcmp(arg, "-b") == 0)
    {
        /* emit expression-level profiling code; see bprint(1) */
        return 1;
    }

    if (strcmp(arg, "-g") == 0)
    {
        /* produce symbol table information for debuggers */
        return 1;
    }

    if (strcmp(arg, "-static") == 0)
    {
        /* specify static libraries (default is dynamic) */
        return 1;
    }

    if (strcmp(arg, "-dynamic") == 0)
    {
        /* specify dynamic libraries*/
        return 1;
    }

    /* unrecognized option */
    return 0;
}
