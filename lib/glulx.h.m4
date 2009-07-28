#ifndef GLULX_H_INCLUDED
#define GLULX_H_INCLUDED

/* Low-level Glulx utility functions. */

/* Gestalt selectors (for use with glulx_gestalt) */
#define glulx_GlulxVersion     0
#define glulx_TerpVersion      1
#define glulx_ResizeMem        2
#define glulx_Undo             3
#define glulx_IOSystem         4
#define glulx_Unicode          5
#define glulx_MemCopy          6
#define glulx_MAlloc           7
#define glulx_MAllocHeap       8
#define glulx_Acceleration     9
#define glulx_AccelFunc       10

union glulx_TypeId
{
    char c[4];
    unsigned u;
};

struct glulx_Version
{
    unsigned major : 16, minor : 8, revision : 8;
};

struct glulx_Header
{
    union glulx_TypeId      magic;
    struct glulx_Version    version;
    unsigned                ramstart;
    unsigned                extstart;
    unsigned                endmem;
    unsigned                stack_size;
    unsigned                start_func;
    unsigned                decoding_tbl;
    unsigned                checksum;
};

extern const struct glulx_Header *const glulx_header;

/* native Glulx operations */
define(`func', `extern `$1' `$2'(ifelse(`$3', `', `void', `$3'));')dnl
include(`glulx-operations.m4')dnl

#endif /* ndef GLULX_H_INCLUDED */
