#ifndef STDARG_H_INCLUDED
#define STDARG_H_INCLUDED

typedef int* va_list;

#define va_start(ap, last) ((void)((ap) = (int*)&(last)))
#define va_arg(ap, type) (*(type*)(--(ap)))
#define va_end(ap) ((void)(ap = NULL))
#define va_copy(aq, ap) ((void)((aq) = (ap)))

#endif /* ndef STDARG_H_INCLUDED */
