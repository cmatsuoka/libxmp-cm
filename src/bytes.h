#ifndef LIBXMP_BYTES_H
#define LIBXMP_BYTES_H

#include <setjmp.h>
#include "types.h"
#include "exception.h"

#define LIBXMP_BYTES_ERANGE	(-100)
#define LIBXMP_BYTES_EINVAL	(-101)
#define LIBXMP_BYTES_EPARMS	(-102)

#define LIBXMP_BYTES_SEEK_SET	0
#define LIBXMP_BYTES_SEEK_CUR	1
#define LIBXMP_BYTES_SEEK_END	2

typedef struct libxmp_bytes__ {
	LIBXMP_EXCEPTION ex;
} *LIBXMP_BYTES;

LIBXMP_BYTES	libxmp_bytes_new	(LIBXMP_EXCEPTION, void *, size_t);
void		libxmp_bytes_release	(LIBXMP_BYTES);
int		libxmp_bytes_left	(LIBXMP_BYTES);
int		libxmp_bytes_scan	(LIBXMP_BYTES, char *, ...);
int		libxmp_bytes_try_read	(LIBXMP_BYTES, void *, int);
void		libxmp_bytes_read	(LIBXMP_BYTES, void *, int);
void		libxmp_bytes_seek	(LIBXMP_BYTES, long, int);
long		libxmp_bytes_tell	(LIBXMP_BYTES);
long		libxmp_bytes_size	(LIBXMP_BYTES);
uint8		libxmp_bytes_read8	(LIBXMP_BYTES);
uint16		libxmp_bytes_read16l	(LIBXMP_BYTES);
uint16		libxmp_bytes_read16b	(LIBXMP_BYTES);
uint32		libxmp_bytes_read24l	(LIBXMP_BYTES);
uint32		libxmp_bytes_read24b	(LIBXMP_BYTES);
uint32		libxmp_bytes_read32l	(LIBXMP_BYTES);
uint32		libxmp_bytes_read32b	(LIBXMP_BYTES);

#endif
