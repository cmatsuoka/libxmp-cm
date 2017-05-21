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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "mem.h"
#include "debug.h"

#define uthash_fatal(msg) goto err_uthash
#include "uthash.h"

struct mem_item {
	void *ptr;
	UT_hash_handle hh;
};

LIBXMP_MEM libxmp_mem_new()
{
	struct libxmp_mem__ *m;

	if ((m = calloc(sizeof (struct libxmp_mem__), 1)) == NULL) {
		return NULL;
	}

	D_(D_WARN "MEM=%p", m);

	return (LIBXMP_MEM)m;
}

void libxmp_mem_release(LIBXMP_MEM mem)
{
	D_(D_WARN "RELEASE MEM=%p", mem);

	libxmp_mem_clear(mem);
	free(mem);
}

void libxmp_mem_throw(LIBXMP_MEM mem, int val, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(mem->_err, LIBXMP_MEM_ERRSIZE, fmt, ap);
	va_end(ap);

	longjmp(mem->jmp, val);
}

void *libxmp_mem_calloc(LIBXMP_MEM mem, size_t size)
{
	void *ptr;

	ptr = libxmp_mem_alloc(mem, size);
	memset(ptr, 0, size);

	return ptr;
}

void *libxmp_mem_alloc(LIBXMP_MEM mem, size_t size)
{
	struct mem_item *head = (struct mem_item *)mem->hash;
	struct mem_item *item;

	D_(D_INFO "size=%ld (hash=%p)", (long)size, mem->hash);

	if ((item = malloc(sizeof (struct mem_item))) == NULL) {
		goto err;
	}

	if ((item->ptr = malloc(size)) == NULL) {
		goto err2;
	}

	HASH_ADD_PTR(head, ptr, item);

	D_(D_INFO "ptr=%p", item->ptr);

	return item->ptr;

    err_uthash:
    err2:
	free(item);
    err:
	libxmp_mem_throw(mem, LIBXMP_MEM_ENOMEM, "%s:%d: memory allocation error (size %ld)",
		__FUNCTION__, __LINE__, (long)size);
	return NULL;
}

void libxmp_mem_free(LIBXMP_MEM mem, void *ptr)
{
	struct mem_item *head = (struct mem_item *)mem->hash;
	struct mem_item *item;

	D_(D_INFO "ptr=%p", ptr);

	HASH_FIND_PTR(head, &ptr, item);
	HASH_DEL(head, item);

	free(item->ptr);
	free(item);
}

void libxmp_mem_clear(LIBXMP_MEM mem)
{
	struct mem_item *head = (struct mem_item *)mem->hash;
	struct mem_item *item, *tmp;

	D_(D_WARN "clear allocations (ptr=%p)", mem->hash);
	
	HASH_ITER(hh, (struct mem_item *)mem->hash, item, tmp) {
		D_(D_INFO "free %p (item %p)", item->ptr, item);
		HASH_DEL(head, item);
		free(item->ptr);
		free(item);
	}
}
