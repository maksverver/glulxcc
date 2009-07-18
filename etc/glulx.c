/* any platform with Glulx back-end */

#include <string.h>

#ifndef LCCDIR
#define LCCDIR "/usr/local/lib/lcc/"
#endif

char *suffixes[] = { ".c", ".i", ".ula", ".ulo", ".ulx", 0 };
char inputs[256] = "";
char *cpp[] = { LCCDIR "cpp", "-Dglulx", "-D__glulx__", "$1", "$2", "$3", 0 };
char *include[] = {"-I" LCCDIR "include/glulx", 0 };
char *com[] = { LCCDIR "rcc", "-target=glulx", "$1", "$2", "$3", 0 };
char *as[] = { "/bin/cp", "--", "$1", "$2", "$3", 0 };
char *ld[] = { LCCDIR "glulxa", "-i", "$2", "-o", "$3", 0 };

int option(char *arg)
{
    return 0;
}
