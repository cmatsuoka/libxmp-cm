#ifndef LIBXMP_ALLOC_H
#define LIBXMP_ALLOC_H

#include <setjmp.h>

#define LIBXMP_MM_ENOMEM	(-1)
#define LIBXMP_MM_EPARAM	(-2)

#define LIBXMP_MM_ERRSIZE	80

typedef struct libxmp_mm__ {
	jmp_buf jmp;
	char _err[LIBXMP_MM_ERRSIZE];
} *LIBXMP_MM;

#define libxmp_mm_catch(buf) setjmp((buf)->jmp)
#define libxmp_mm_error(buf) ((buf)->_err)

LIBXMP_MM	libxmp_mm_new		(void);
void		libxmp_mm_release	(LIBXMP_MM);
void		libxmp_mm_throw		(LIBXMP_MM, int, char *, ...);
void		*libxmp_mm_calloc	(LIBXMP_MM, size_t);
void		*libxmp_mm_alloc	(LIBXMP_MM, size_t);
void		*libxmp_mm_realloc	(LIBXMP_MM, void *, size_t);
void		libxmp_mm_free		(LIBXMP_MM, void *);
void		libxmp_mm_clear		(LIBXMP_MM);

#endif
