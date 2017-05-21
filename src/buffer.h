#ifndef LIBXMP_BUFFER_H
#define LIBXMP_BUFFER_H

#include <setjmp.h>
#include "types.h"

typedef struct libxmp_buffer_opaque{} *LIBXMP_BUFFER;

LIBXMP_BUFFER	libxmp_buffer_new	(void *, size_t);
void		libxmp_buffer_release	(LIBXMP_BUFFER);
char		*libxmp_buffer_catch	(LIBXMP_BUFFER);
int		libxmp_buffer_left	(LIBXMP_BUFFER);
int		libxmp_buffer_scan	(LIBXMP_BUFFER, char *, ...);
int		libxmp_buffer_try_read	(LIBXMP_BUFFER, void *, int);
void		libxmp_buffer_read	(LIBXMP_BUFFER, void *, int);
void		libxmp_buffer_seek	(LIBXMP_BUFFER, long, int);
long		libxmp_buffer_tell	(LIBXMP_BUFFER);
long		libxmp_buffer_size	(LIBXMP_BUFFER);
uint8		libxmp_buffer_read8	(LIBXMP_BUFFER);
uint16		libxmp_buffer_read16l	(LIBXMP_BUFFER);
uint16		libxmp_buffer_read16b	(LIBXMP_BUFFER);
uint32		libxmp_buffer_read24l	(LIBXMP_BUFFER);
uint32		libxmp_buffer_read24b	(LIBXMP_BUFFER);
uint32		libxmp_buffer_read32l	(LIBXMP_BUFFER);
uint32		libxmp_buffer_read32b	(LIBXMP_BUFFER);

#endif
