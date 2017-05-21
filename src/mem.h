#ifndef LIBXMP_ALLOC_H
#define LIBXMP_ALLOC_H

#include <setjmp.h>


typedef struct libxmp_mem_opaque{} *LIBXMP_MEM;

LIBXMP_MEM	libxmp_mem_new		(void);
void		libxmp_mem_release	(LIBXMP_MEM);
char		*libxmp_mem_catch	(LIBXMP_MEM);
void		*libxmp_mem_calloc	(LIBXMP_MEM, size_t);
void		*libxmp_mem_alloc	(LIBXMP_MEM, size_t);
void		libxmp_mem_free		(LIBXMP_MEM, void *);
void		libxmp_mem_clear	(LIBXMP_MEM);

#endif
