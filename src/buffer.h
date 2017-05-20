#ifndef LIBXMP_BUFFER_H
#define LIBXMP_BUFFER_H

#include <setjmp.h>
#include "types.h"

#define LIBXMP_BUFFER_ERANGE (-1)
#define LIBXMP_BUFFER_EINVAL (-2)
struct libxmp_buffer {
	jmp_buf jmp;
	uint8 *start;
	uint8 *pos;
	uint8 *end;
};

#endif
