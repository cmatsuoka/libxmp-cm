#ifndef LIBXMP_DEBUG_H
#define LIBXMP_DEBUG_H

#ifdef _MSC_VER

#   define D_CRIT "  Error: "
#   define D_WARN "Warning: "
#   define D_INFO "   Info: "
#   ifndef CLIB_DECL
#       define CLIB_DECL
#   endif
#   ifdef DEBUG
#       ifndef ATTR_PRINTF
#           define ATTR_PRINTF(x,y)
#       endif
void CLIB_DECL D_(const char *text, ...) ATTR_PRINTF(1,2);
#   else
// VS prior to VC7.1 does not support variadic macros. VC8.0 does not optimize unused parameters passing
#       if _MSC_VER < 1400
void __inline CLIB_DECL D_(const char *text, ...) { do {} while (0); }
#       else
#           define D_(args, ...) do {} while (0)
#       endif
#   endif

#elif defined __ANDROID__

#   ifdef DEBUG
#       include <android/log.h>
#       define D_CRIT "  Error: "
#       define D_WARN "Warning: "
#       define D_INFO "   Info: "
#       define D_(args...) do { \
	    __android_log_print(ANDROID_LOG_DEBUG, "libxmp", args); \
	} while (0)
#   else
#       define D_(args...) do {} while (0)
#   endif

#elif defined(__WATCOMC__)

#   ifdef DEBUG
#       define D_INFO "\x1b[33m"
#       define D_CRIT "\x1b[31m"
#       define D_WARN "\x1b[36m"
#       define D_(...) do { \
	    printf("\x1b[33m%s \x1b[37m[%s:%d] " D_INFO, __FUNCTION__, \
		__FILE__, __LINE__); printf (__VA_ARGS__); printf ("\x1b[0m\n"); \
	} while (0)
#   else
#       define D_(...) do {} while (0)
#   endif

#else

#   ifdef DEBUG
#       define D_INFO "\x1b[33m"
#       define D_CRIT "\x1b[31m"
#       define D_WARN "\x1b[36m"
#       define D_(args...) do { \
	    printf("\x1b[33m%s \x1b[37m[%s:%d] " D_INFO, __FUNCTION__, \
		__FILE__, __LINE__); printf (args); printf ("\x1b[0m\n"); \
	} while (0)
#   else
#       define D_(args...) do {} while (0)
#   endif

#endif	/* !_MSC_VER */

#endif
