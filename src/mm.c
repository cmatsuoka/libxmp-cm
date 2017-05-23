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
#include "mm.h"
#include "debug.h"

#define uthash_fatal(msg) goto err_uthash
#include "uthash.h"

struct mem_item {
	void *ptr;
	UT_hash_handle hh;
};

struct libxmp_mm {
	jmp_buf jmp;
	char _err[LIBXMP_MM_ERRSIZE];
	struct mem_item *hash;
};

LIBXMP_MM libxmp_mm_new()
{
	struct libxmp_mm *m;

	if ((m = calloc(sizeof (struct libxmp_mm), 1)) == NULL) {
		return NULL;
	}

	D_(D_WARN "MEM=%p", m);

	return (LIBXMP_MM)m;
}

void libxmp_mm_release(LIBXMP_MM m__)
{
	D_(D_WARN "RELEASE MEM=%p", m__);

	libxmp_mm_clear(m__);
	free(m__);
}

void libxmp_mm_throw(LIBXMP_MM m__, int val, char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(m__->_err, LIBXMP_MM_ERRSIZE, fmt, ap);
	va_end(ap);

	longjmp(m__->jmp, val);
}

void *libxmp_mm_calloc(LIBXMP_MM m__, size_t size)
{
	void *ptr;

	ptr = libxmp_mm_alloc(m__, size);
	memset(ptr, 0, size);

	return ptr;
}

void *libxmp_mm_alloc(LIBXMP_MM m__, size_t size)
{
	struct libxmp_mm *mem = (struct libxmp_mm *)m__;
	struct mem_item *item;

	D_(D_INFO "size=%ld (hash=%p)", (long)size, mem->hash);

	if ((item = malloc(sizeof (struct mem_item))) == NULL) {
		goto err;
	}

	if ((item->ptr = malloc(size)) == NULL) {
		goto err2;
	}

	HASH_ADD_PTR(mem->hash, ptr, item);

	D_(D_INFO "ptr=%p", item->ptr);

	return item->ptr;

    err_uthash:
    err2:
	free(item);
    err:
	libxmp_mm_throw(m__, LIBXMP_MM_ENOMEM, "%s:%d: memory allocation error (size %ld)",
		__FUNCTION__, __LINE__, (long)size);
	return NULL;
}

void *libxmp_mm_realloc(LIBXMP_MM m__, void *ptr, size_t size)
{
	struct libxmp_mm *mem = (struct libxmp_mm *)m__;
	struct mem_item *item;

	D_(D_INFO "size=%ld (hash=%p)", (long)size, mem->hash);

	HASH_FIND_PTR(mem->hash, &ptr, item);

	if ((item->ptr = realloc(item->ptr, size)) == NULL) {
		goto err;
	}

	D_(D_INFO "ptr=%p", item->ptr);

	return item->ptr;

    err:
	libxmp_mm_throw(m__, LIBXMP_MM_ENOMEM, "%s:%d: memory reallocation error (size %ld)",
		__FUNCTION__, __LINE__, (long)size);
	return NULL;
}

void libxmp_mm_free(LIBXMP_MM m__, void *ptr)
{
	struct libxmp_mm *mem = (struct libxmp_mm *)m__;
	struct mem_item *item;

	D_(D_INFO "ptr=%p", ptr);

	HASH_FIND_PTR(mem->hash, &ptr, item);
	HASH_DEL(mem->hash, item);

	free(item->ptr);
	free(item);
}

void libxmp_mm_clear(LIBXMP_MM m__)
{
	struct libxmp_mm *mem = (struct libxmp_mm *)m__;
	struct mem_item *item, *tmp;

	D_(D_WARN "clear allocations (ptr=%p)", mem->hash);
	
	HASH_ITER(hh, mem->hash, item, tmp) {
		D_(D_INFO "free %p (item %p)", item->ptr, item);
		HASH_DEL(mem->hash, item);
		free(item->ptr);
		free(item);
	}
}
