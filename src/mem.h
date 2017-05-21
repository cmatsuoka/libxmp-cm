#ifndef LIBXMP_ALLOC_H
#define LIBXMP_ALLOC_H

#include <setjmp.h>

#define LIBXMP_MEM_ENOMEM (-1)

struct libxmp_mem {
	void *hash;
	jmp_buf jmp;
};

struct libxmp_mem *libxmp_mem_new(void);
void	libxmp_mem_release	(struct libxmp_mem *);
void	*libxmp_mem_calloc	(struct libxmp_mem *, size_t);
void	*libxmp_mem_alloc	(struct libxmp_mem *, size_t);
void	libxmp_mem_free		(struct libxmp_mem *, void *);
void	libxmp_mem_clear	(struct libxmp_mem *);

#endif
