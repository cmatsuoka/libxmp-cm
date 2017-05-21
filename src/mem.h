#ifndef LIBXMP_ALLOC_H
#define LIBXMP_ALLOC_H

#include <setjmp.h>

#define LIBXMP_MEM_ENOMEM	(-1)
#define LIBXMP_MEM_EPARAM	(-2)

#define LIBXMP_MEM_ERRSIZE	80

typedef struct libxmp_mem__ {
	jmp_buf jmp;
	char _err[LIBXMP_MEM_ERRSIZE];
} *LIBXMP_MEM;

#define libxmp_mem_catch(buf) setjmp((buf)->jmp)
#define libxmp_mem_error(buf) ((buf)->_err)

LIBXMP_MEM	libxmp_mem_new		(void);
void		libxmp_mem_release	(LIBXMP_MEM);
void		libxmp_mem_throw	(LIBXMP_MEM, int, char *, ...);
void		*libxmp_mem_calloc	(LIBXMP_MEM, size_t);
void		*libxmp_mem_alloc	(LIBXMP_MEM, size_t);
void		*libxmp_mem_realloc	(LIBXMP_MEM, void *, size_t);
void		libxmp_mem_free		(LIBXMP_MEM, void *);
void		libxmp_mem_clear	(LIBXMP_MEM);

#endif
