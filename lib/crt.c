#include "stdio.h"
#include "glk.h"

static int argc = 1;
static char *argv[2] = { "main", NULL };

void _crt_start()
{
    winid_t win;
    strid_t str;
    int status;

    /* Open main Glk window and set the current output stream */
    win = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
    str = (win == NULL) ? NULL : glk_window_get_stream(win);
    glk_stream_set_current(str);

    /* Initialize stdin/stdout/stderr */
    stdin  = NULL;
    stdout = str;
    stderr = str;

    /* Call the real main function */
    status = main(argc, argv);
    exit(status);
}
