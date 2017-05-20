#ifndef LIBXMP_BUFFER_H
#define LIBXMP_BUFFER_H

#include <setjmp.h>
#include "types.h"

#define LIBXMP_BUFFER_ERANGE  (-1)
#define LIBXMP_BUFFER_EINVAL  (-2)
#define LIBXMP_BUFFER_EFORMAT (-3)

struct libxmp_buffer {
	jmp_buf jmp;
	uint8 *start;
	uint8 *pos;
	uint8 *end;
};

#define LIBXMP_BUFFER_SIZE(b) (b->end - b->start)

struct libxmp_buffer	*libxmp_buffer_new(unsigned char *, size_t);
void	libxmp_buffer_release	(struct libxmp_buffer *);
int	libxmp_buffer_left	(struct libxmp_buffer *);
int	libxmp_buffer_scan	(struct libxmp_buffer *, char *, ...);
int	libxmp_buffer_read	(struct libxmp_buffer *, void *, int);
void	libxmp_buffer_seek	(struct libxmp_buffer *, long, int);
long	libxmp_buffer_tell	(struct libxmp_buffer *);
uint8	libxmp_buffer_read8	(struct libxmp_buffer *);
uint16	libxmp_buffer_read16l	(struct libxmp_buffer *);
uint16	libxmp_buffer_read16b	(struct libxmp_buffer *);
uint32	libxmp_buffer_read24l	(struct libxmp_buffer *);
uint32	libxmp_buffer_read24b	(struct libxmp_buffer *);
uint32	libxmp_buffer_read32l	(struct libxmp_buffer *);
uint32	libxmp_buffer_read32b	(struct libxmp_buffer *);

#endif
