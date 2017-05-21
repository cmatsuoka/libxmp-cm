#ifndef LIBXMP_ALLOC_H
#define LIBXMP_ALLOC_H

#include <setjmp.h>

#define LIBXMP_MEM_ENOMEM	(-1)
#define LIBXMP_MEM_EPARAM	(-2)

typedef struct libxmp_mem_opaque__{} *LIBXMP_MEM;

LIBXMP_MEM	libxmp_mem_new		(void);
void		libxmp_mem_release	(LIBXMP_MEM);
int		libxmp_mem_catch	(LIBXMP_MEM, char **);
void		libxmp_mem_throw	(LIBXMP_MEM, int, char *, ...);
void		*libxmp_mem_calloc	(LIBXMP_MEM, size_t);
void		*libxmp_mem_alloc	(LIBXMP_MEM, size_t);
void		libxmp_mem_free		(LIBXMP_MEM, void *);
void		libxmp_mem_clear	(LIBXMP_MEM);

#endif
