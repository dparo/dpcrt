#ifndef HGUARD_053e391b341240cd8df8893eb826d004
#define HGUARD_053e391b341240cd8df8893eb826d004

#include "dpcrt_utils.h"


__BEGIN_DECLS


#ifndef _VA_LIST
#define _VA_LIST
typedef __builtin_va_list va_list;
#endif

# if __GNUC__ || __clang__
#  ifndef va_start
#    define va_start(ap, last)   __builtin_va_start(ap, last)
#  endif
#  ifndef va_arg
#    define va_arg(ap, type)     __builtin_va_arg(ap, type)
#  endif
#  ifndef va_end
#    define va_end(ap)           __builtin_va_end(ap)
#  endif
#  ifndef va_copy
#    define va_copy(dest, src)   __builtin_va_copy(dest, src)
#  endif
#else
#  error "Check for MSVC what is the correct thing to put here"
#endif


__END_DECLS

#endif /* HGUARD_053e391b341240cd8df8893eb826d004 */
