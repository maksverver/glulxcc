#ifndef GLULX_H_INCLUDED
#define GLULX_H_INCLUDED

/* Glulx-specific functions go here.
   You probably shouldn't use these directly as they're not portable. */

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

void glulx_quit();
void glulx_debugtrap(int val);
int glulx_random(int range);
void glulx_setrandom(unsigned seed);
int glulx_gestalt(int selector, int argument);

#endif /* ndef GLULX_H_INCLUDED */
