#ifndef STDARG_H
#define STDARG_H

/* implementation of stdarg.h, shamelessly ripped from linux 0.01 */

typedef char *va_list;

#define __va_rounded_size(TYPE) \
	(((sizeof(TYPE) + sizeof(long) - 1) / sizeof(long)) * sizeof(long))

#define va_start(AP, LASTARG) \
	(AP = ((char *) &(LASTARG) + __va_rounded_size(LASTARG)))

#define va_end(AP) ((void)0)

#define va_arg(AP, TYPE) \
	(AP += __va_rounded_size(TYPE), \
	*((TYPE *) (AP - __va_rounded_size(TYPE))))

#endif /* STDARG_H */
