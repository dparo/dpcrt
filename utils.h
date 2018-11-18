/*
 * Copyright (C) 2018  Davide Paro
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

#ifndef HGUARD_183f4851c423485faffa8b8cd1f85f24
#define HGUARD_183f4851c423485faffa8b8cd1f85f24

#include "../build_config.h"
#include "compiler.h"
#include "types.h"
#include <errno.h>
#include <string.h>

__BEGIN_DECLS

#ifndef NULL
#  define NULL ((void*) 0x0)
#endif


#define cast(T, expr) (T) (expr)

#if defined __DEBUG
/* Note keep this in one liner to make the __LINE__ macro expansion work */
#  define TEST_FUNCTION(test)                                           \
    static bool CONCAT(___first_time_called___, __LINE__) = false; if ( ((test) == true) && (CONCAT(___first_time_called___, __LINE__) == false) )  { CONCAT(___first_time_called___, __LINE__) = true; goto CONCAT(___exec___, __LINE__); } if (0)CONCAT(___exec___, __LINE__):
#else
#  define TEST_FUNCTION(...)
#endif



#define UNUSED(p) (void)(p)
#define empty_code_path(...) do { } while(0)

// Usefull markers for in and output pointers inside a function
#define __OUT__
#define __IN__

#define ABS(x) ((x) > 0 ? (x) : -(x))

#define MIN(a, b)     ((a) < (b) ? (a) : (b))
#define MAX(a, b)     ((a) < (b) ? (b) : (a))

#define ALIGN(N, S) ((((N) + (S) - 1) / (S)) * (S))
#define ALIGN_PTR(N, S) ((void*)(ALIGN ( ((size_t)(N)) , S) ))


#define KILOBYTES(x) ((size_t) (x) << 10)
#define MEGABYTES(x) ((size_t) (KILOBYTES(x)) << 10)
#define GIGABYTES(x) ((size_t) (MEGABYTES(x)) << 10)

#define KILO(x) ((size_t) (x) << 10)
#define MEGA(x) ((size_t) (KILO(x)) << 10)
#define GIGA(x) ((size_t) (MEGA(x)) << 10)




# ifndef PAGE_SHIFT
#   define PAGE_SHIFT      12
# endif

# ifndef PAGE_SIZE
#   define PAGE_SIZE (KILOBYTES(4))
# endif

# ifndef PAGE_MASK
#   define PAGE_MASK       (~(PAGE_SIZE - 1))
# endif

# ifndef PAGE_ALIGN
#   define PAGE_ALIGN(addr)  (ALIGN(addr, PAGE_SIZE))
# endif


#define IS_POW2(x) (((x) & ((x) - 1)) == 0)



#define memclr(SRC, SIZE)                       \
    (memset((SRC), 0, (SIZE)))


__END_DECLS

#endif  /* HGUARD_183f4851c423485faffa8b8cd1f85f24 */
