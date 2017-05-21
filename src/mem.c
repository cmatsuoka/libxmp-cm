#include <stdlib.h>
#include "mem.h"

#define uthash_fatal(msg) goto err_uthash
#include "uthash.h"

struct mem_item {
	void *ptr;
	UT_hash_handle hh;
};

struct libxmp_mem *libxmp_mem_new()
{
	struct libxmp_mem *mem;

	if ((mem = malloc(sizeof (struct libxmp_mem))) == NULL) {
		return NULL;
	}

	mem->hash = NULL;

	return mem;
}

void libxmp_mem_release(struct libxmp_mem *mem)
{
	libxmp_mem_clear(mem);
	free(mem);
}

void *libxmp_mem_calloc(struct libxmp_mem *mem, size_t size)
{
	void *ptr;

	ptr = libxmp_mem_alloc(mem, size);
	memset(ptr, 0, size);

	return ptr;
}

void *libxmp_mem_alloc(struct libxmp_mem *mem, size_t size)
{
	struct mem_item *head = (struct mem_item *)mem->hash;
	struct mem_item *item;

	if ((item = malloc(sizeof (struct mem_item))) == NULL) {
		goto err;
	}

	if ((item->ptr = malloc(size)) == NULL) {
		goto err2;
	}

	HASH_ADD_PTR(head, ptr, item);

	return item->ptr;

    err_uthash:
    err2:
	free(item);
    err:
	longjmp(mem->jmp, LIBXMP_MEM_ENOMEM);
	return NULL;
}

void libxmp_mem_free(struct libxmp_mem *mem, void *ptr)
{
	struct mem_item *head = (struct mem_item *)mem->hash;
	struct mem_item *item;

	HASH_FIND_PTR(head, &ptr, item);
	HASH_DEL(head, item);

	free(item->ptr);
	free(item);
}

void libxmp_mem_clear(struct libxmp_mem *mem)
{
	struct mem_item *head = (struct mem_item *)mem->hash;
	struct mem_item *item, *tmp;
	
	HASH_ITER(hh, head, item, tmp) {
		HASH_DEL(head, item);
		free(item->ptr);
		free(item);
	}
}
