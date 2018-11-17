/*
 * Copyright (C) 2018  Davide Paro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UTILS_H
#define UTILS_H

#include "../build_config.h"
#include "compiler.h"
#include "types.h"
#include <errno.h>
#include <string.h>

__BEGIN_DECLS

#ifndef NULL
#define NULL ((void*) 0x0)
#endif


#define cast(T, expr) (T) (expr)

#if defined __DEBUG
/* Note keep this in one liner to make the __LINE__ macro expansion work */
#define TEST_FUNCTION(test)                                             \
    static bool CONCAT(___first_time_called___, __LINE__) = false; if ( ((test) == true) && (CONCAT(___first_time_called___, __LINE__) == false) )  { CONCAT(___first_time_called___, __LINE__) = true; goto CONCAT(___exec___, __LINE__); } if (0)CONCAT(___exec___, __LINE__):
#else
#define TEST_FUNCTION(...)
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
#    define PAGE_SHIFT      12
#endif

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

#endif
