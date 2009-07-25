#ifndef GLULX_H_INCLUDED
#define GLULX_H_INCLUDED

/* Glulx-specific functions go here.
   You probably shouldn't use these directly. */

void glulx_quit();
void glulx_debugtrap(int val);
int glulx_random(int range);
void glulx_setrandom(unsigned seed);

#endif /* ndef GLULX_H_INCLUDED */
