#ifndef LIBXMP_FORMAT_H
#define LIBXMP_FORMAT_H

#include <stdio.h>
#include "common.h"
#include "bytes.h"
#include "mm.h"

struct format_loader {
	const char *name;
	int (*const test)(LIBXMP_MM, LIBXMP_BYTES, char *, const int);
	int (*const loader)(LIBXMP_MM, LIBXMP_BYTES, struct module_data *, const int);
};

char **format_list(void);

#ifndef LIBXMP_CORE_PLAYER

#define NUM_FORMATS 52
#define NUM_PW_FORMATS 43

//int pw_test_format(HIO_HANDLE *, char *, const int, struct xmp_test_info *);
#endif

#endif

