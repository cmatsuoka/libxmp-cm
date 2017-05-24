#ifndef LIBXMP_MM_H
#define LIBXMP_MM_H

#include <setjmp.h>
#include "exception.h"

#define LIBXMP_MM_ENOMEM	(-200)
#define LIBXMP_MM_EPARMS	(-201)

typedef struct libxmp_mm__ {
	LIBXMP_EXCEPTION ex;
} *LIBXMP_MM;

LIBXMP_MM	libxmp_mm_new		(LIBXMP_EXCEPTION);
void		libxmp_mm_release	(LIBXMP_MM);
void		*libxmp_mm_calloc	(LIBXMP_MM, size_t);
void		*libxmp_mm_alloc	(LIBXMP_MM, size_t);
void		*libxmp_mm_realloc	(LIBXMP_MM, void *, size_t);
void		libxmp_mm_free		(LIBXMP_MM, void *);
void		libxmp_mm_clear		(LIBXMP_MM);

#endif
