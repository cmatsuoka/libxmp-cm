/* Extended Module Player
 * Copyright (C) 1996-2016 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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
