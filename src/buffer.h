#ifndef LIBXMP_BUFFER_H
#define LIBXMP_BUFFER_H

#include <setjmp.h>
#include "types.h"

#define LIBXMP_BUFFER_EINVAL	(-1)
#define LIBXMP_BUFFER_ERANGE	(-2)
#define LIBXMP_BUFFER_EPARM	(-3)

#define LIBXMP_BUFFER_ERRSIZE	80

struct libxmp_buffer__ {
	jmp_buf jmp;
	uint8 *start;
	uint8 *pos;
	uint8 *end;
	char _err[LIBXMP_BUFFER_ERRSIZE];
};

typedef struct libxmp_buffer__ *LIBXMP_BUFFER;

#define libxmp_buffer_catch(buf) setjmp((buf)->jmp)
#define libxmp_buffer_error(buf) ((buf)->_err)

LIBXMP_BUFFER	libxmp_buffer_new	(void *, size_t);
void		libxmp_buffer_release	(LIBXMP_BUFFER);
void		libxmp_buffer_throw	(LIBXMP_BUFFER, int, char *, ...);
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
