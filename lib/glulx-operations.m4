dnl Only includes opcodes that are not available directly in the language, and
dnl that do not interfere with normal execution of the program (for example,
dnl functions that manipulate the stack have been omitted).
dnl
dnl Array Data
func(`void',        `astorebit',    `void *start, unsigned index, int value')
func(`int',         `aloadbit',     `void *start, unsigned index')
dnl
dnl Memory Map
func(`unsigned',    `getmemsize',  `')
func(`int',         `setmemsize',  `unsigned size')
dnl
dnl Memory Allocation Heap
func(`void*',       `malloc',       `unsigned size')
func(`void',        `mfree',        `void *ptr')
dnl
dnl Game State
func(`void',        `quit',         `')
func(`void',        `restart',      `')
func(`int',         `save',         `void *stream')
func(`int',         `restore',      `void *stream')
func(`int',         `saveundo',     `')
func(`int',         `restoreundo',  `')
func(`void',        `protect',      `void *start, unsigned size')
func(`int',         `verify',       `')
dnl
dnl Random Number Generator
func(`int',         `random',       `int range')
func(`void',        `setrandom',    `unsigned seed')
dnl
dnl Block Copy and Clear
func(`void',        `mzero',        `unsigned size, void *dest')
func(`void',        `mcopy',        `unsigned size, void *src, void *dest')
dnl
dnl Search
dnl TODO: linearsearch, binarysearch, linkedsearch
dnl
dnl Miscellaneous
func(`int',         `gestalt',      `int selector, int argument')
func(`void',        `debugtrap',    `int value')
dnl EOF
