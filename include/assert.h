extern void __assert_failed(const char *file, int line, const char *expr);

#ifdef assert
#undef assert
#endif

#ifdef NDEBUG
#define assert(c) ((void)(c))
#else
#define assert(c) ((c) ? (void)0 : __assert_failed(__FILE__, __LINE__, #c))
#endif 
