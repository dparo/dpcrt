/*
 * Copyright (C) 2019  Davide Paro
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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
