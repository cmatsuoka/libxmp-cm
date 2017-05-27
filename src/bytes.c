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
#include <string.h>
#include <stdarg.h>
#include "bytes.h"
#include "debug.h"

struct libxmp_bytes {
	LIBXMP_EXCEPTION ex;
	uint8 *start;
	uint8 *pos;
	uint8 *end;
};

#define B(b) ((struct libxmp_bytes *)(b))

LIBXMP_BYTES libxmp_bytes_new(LIBXMP_EXCEPTION ex, void *p, size_t size)
{
	struct libxmp_bytes *b;

	if ((b = calloc(sizeof (struct libxmp_bytes), 1)) == NULL) {
		goto err;
	}

	D_(D_WARN "BUF=%p", b);
	b->ex = ex;
	b->start = b->pos = p;
	b->end = b->start + size;

	return (LIBXMP_BYTES)b;

    err:
	return NULL;
}

void libxmp_bytes_release(LIBXMP_BYTES buf)
{
	D_(D_WARN "RELEASE BUF=%p", buf);
	free(buf);
}

#define ASSIGN_ENDIAN(buf,T,fmt,f) do {					\
	char e = *(fmt)++;						\
	T *b = va_arg(ap, T *);						\
	if (b == NULL) {						\
		B(buf)->pos += sizeof (T);				\
	} else {							\
		if (e == 'l') {						\
			*b = (T)libxmp_bytes_read##f##l(buf);		\
		} else if (e == 'b') {					\
			*b = (T)libxmp_bytes_read##f##b(buf);		\
		} else {						\
			libxmp_exception_throw(B(buf)->ex, LIBXMP_BYTES_EINVAL,\
				"%s:%d: invalid endian: %c", __FUNCTION__, __LINE__, e);	\
		}							\
	}								\
} while (0)

#define ASSIGN_VALUE(buf,T,fmt,size) do {			\
	switch (size) {						\
	case 8:							\
		{ T *b = va_arg(ap, T *);			\
		if (b == NULL) {				\
			B(buf)->pos++;				\
		} else {					\
			*b = (T)libxmp_bytes_read8(buf);	\
		}						\
		break; }					\
	case 16:						\
		ASSIGN_ENDIAN((buf), T, (fmt), 16);		\
		break;						\
	case 24:						\
		ASSIGN_ENDIAN((buf), T, (fmt), 24);		\
		break;						\
	case 32:						\
		ASSIGN_ENDIAN((buf), T, (fmt), 32);		\
		break;						\
	}							\
} while (0)

int libxmp_bytes_left(LIBXMP_BYTES buf)
{
	return B(buf)->end - B(buf)->pos;
}

int libxmp_bytes_scan(LIBXMP_BYTES buf, char *fmt, ...)
{
	va_list ap;
	int size;
	char t;
	void *p;

	va_start(ap, fmt);

	D_(D_INFO "buffer scan: %s", fmt);

	while ((t = *fmt) != 0) {
		switch (*fmt++) {
		case 's':
			size = strtoul(fmt, &fmt, 10);
			p = va_arg(ap, void*);
			if (p == NULL) {
				B(buf)->pos += size;
			} else {
				libxmp_bytes_read(buf, p, size);
			}
			break;
		case 'b':
			size = strtoul(fmt, &fmt, 10);
			ASSIGN_VALUE(buf, uint8, fmt, size);
			break;
		case 'w':
			size = strtoul(fmt, &fmt, 10);
			ASSIGN_VALUE(buf, uint16, fmt, size);
			break;
		case 'd':
			size = strtoul(fmt, &fmt, 10);
			ASSIGN_VALUE(buf, uint32, fmt, size);
			break;
		default:
			D_(D_CRIT "invalid type %c", t);
			goto err;
		}

		if (*fmt == ';') {
			fmt++;
		} else if (*fmt != 0) {
			D_(D_CRIT "missing separator");
			goto err;
		}
	}

	va_end(ap);
	return 0;

    err:
	va_end(ap);
	return -1;
}

void libxmp_bytes_read(LIBXMP_BYTES buf, void *dst, int size)
{
	D_(D_INFO "pos=%ld size=%d end=%ld", B(buf)->pos - B(buf)->start, size, B(buf)->end - B(buf)->start);
	/* check range */
	if (B(buf)->pos + size > B(buf)->end) {
		libxmp_exception_throw(B(buf)->ex, LIBXMP_BYTES_ERANGE, "%s:%d: invalid read (pos %d, size %ld)",
			__FUNCTION__, __LINE__, B(buf)->pos - B(buf)->start, size);
	}

	memcpy(dst, B(buf)->pos, size);
	B(buf)->pos += size;
}

int libxmp_bytes_try_read(LIBXMP_BYTES buf, void *dst, int size)
{
	D_(D_INFO "pos=%ld size=%d end=%ld", B(buf)->pos - B(buf)->start, size, B(buf)->end - B(buf)->start);

	/* check range */
	if (B(buf)->pos + size > B(buf)->end) {
		size = B(buf)->end - B(buf)->pos;
	}

	memcpy(dst, B(buf)->pos, size);
	B(buf)->pos += size;

	return size;
}

#define CHECK_RANGE(b,x) do {					\
	if ((x) >= B(b)->end || (x) < B(b)->start) {		\
		libxmp_exception_throw(B(b)->ex, LIBXMP_BYTES_ERANGE,	\
			"%s:%d: invalid offset %ld (size %ld)", \
			__FUNCTION__, __LINE__, (x)-B(b)->start, B(b)->end-B(b)->start); \
	}							\
	B(b)->pos = (x);						\
} while (0)

void libxmp_bytes_seek(LIBXMP_BYTES buf, long offset, int whence)
{
	switch (whence) {
	case LIBXMP_BYTES_SEEK_SET:
		CHECK_RANGE(buf, B(buf)->start + offset);
		break;
	case LIBXMP_BYTES_SEEK_CUR:
		CHECK_RANGE(buf, B(buf)->pos + offset);
		break;
	case LIBXMP_BYTES_SEEK_END:
		CHECK_RANGE(buf, B(buf)->end - offset - 1);
		break;
	default:
		libxmp_exception_throw(B(buf)->ex, LIBXMP_BYTES_EINVAL, "buffer seek: invalid seek whence %d", whence);
	}
}

long libxmp_bytes_tell(LIBXMP_BYTES buf)
{
	return B(buf)->pos - B(buf)->start;
}

long libxmp_bytes_size(LIBXMP_BYTES buf)
{
	return B(buf)->end - B(buf)->start;
}


#define CHECK_SIZE(b,x) do {					\
	if (B(b)->pos + (x) > B(b)->end) {			\
		libxmp_exception_throw(B(b)->ex, LIBXMP_BYTES_ERANGE,	\
			"%s:%d: invalid position %ld (size %ld)", \
			__FUNCTION__, __LINE__, B(b)->pos-B(b)->start, B(b)->end-B(b)->start);	\
	}							\
} while (0)

uint8 libxmp_bytes_read8(LIBXMP_BYTES buf)
{
	CHECK_SIZE(buf, 1);

	return *B(buf)->pos++;
}

uint16 libxmp_bytes_read16l(LIBXMP_BYTES buf)
{
	uint16 a, b;

	CHECK_SIZE(buf, 2);

	a = (uint16)*B(buf)->pos++;
	b = (uint16)*B(buf)->pos++;

	return (b << 8) | a;
}

uint16 libxmp_bytes_read16b(LIBXMP_BYTES buf)
{
	uint16 a, b;

	CHECK_SIZE(buf, 2);

	a = (uint16)*B(buf)->pos++;
	b = (uint16)*B(buf)->pos++;

	return (a << 8) | b;
}

uint32 libxmp_bytes_read24l(LIBXMP_BYTES buf)
{
	uint32 a, b, c;

	CHECK_SIZE(buf, 3);

	a = (uint32)*B(buf)->pos++;
	b = (uint32)*B(buf)->pos++;
	c = (uint32)*B(buf)->pos++;

	return (c << 16) | (b << 8) | a;
}

uint32 libxmp_bytes_read24b(LIBXMP_BYTES buf)
{
	uint32 a, b, c;

	CHECK_SIZE(buf, 3);

	a = (uint32)*B(buf)->pos++;
	b = (uint32)*B(buf)->pos++;
	c = (uint32)*B(buf)->pos++;

	return (a << 16) | (b << 8) | c;
}

uint32 libxmp_bytes_read32l(LIBXMP_BYTES buf)
{
	uint32 a, b, c, d;

	CHECK_SIZE(buf, 4);

	a = (uint32)*B(buf)->pos++;
	b = (uint32)*B(buf)->pos++;
	c = (uint32)*B(buf)->pos++;
	d = (uint32)*B(buf)->pos++;

	return (d << 24) | (c << 16) | (b << 8) | a;
}

uint32 libxmp_bytes_read32b(LIBXMP_BYTES buf)
{
	uint32 a, b, c, d;

	CHECK_SIZE(buf, 4);

	a = (uint32)*B(buf)->pos++;
	b = (uint32)*B(buf)->pos++;
	c = (uint32)*B(buf)->pos++;
	d = (uint32)*B(buf)->pos++;

	return (a << 24) | (b << 16) | (c << 8) | d;
}

