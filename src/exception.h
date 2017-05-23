#ifndef LIBXMP_EXCEPTION_H
#define LIBXMP_EXCEPTION_H

#include <setjmp.h>

#define LIBXMP_EXCEPTION_ENOMEM	(-1)
#define LIBXMP_EXCEPTION_EPARAM	(-2)

#define LIBXMP_EXCEPTION_ERRSIZE	80

typedef struct libxmp_exception__ {
	jmp_buf jmp;
	char _err[LIBXMP_EXCEPTION_ERRSIZE];
} LIBXMP_EXCEPTION;

#define libxmp_exception_catch(ex) setjmp((ex)->jmp)
#define libxmp_exception_error(ex) ((ex)->_err)

void		libxmp_exception_throw		(LIBXMP_EXCEPTION *, int, char *, ...);

#endif
