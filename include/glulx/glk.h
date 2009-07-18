#ifndef GLK_H_INCLUDED
#define GLK_H_INCLUDED

typedef unsigned glui32;
typedef signed glsi32;

typedef struct glk_window_struct  *winid_t;
typedef struct glk_stream_struct  *strid_t;
typedef struct glk_fileref_struct *frefid_t;
typedef struct glk_schannel_struct *schanid_t;

winid_t glk_window_open(winid_t split, glui32 method, glui32 size,
    glui32 wintype, glui32 rock);
void glk_set_window(winid_t win);
void glk_put_string(char *s);
void glk_put_char(unsigned char ch);

#endif /* ndef GLK_H_INCLUDED */
