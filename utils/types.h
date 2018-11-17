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
#ifndef TYPES_H
#define TYPES_H

#include "compiler.h"

__BEGIN_DECLS

#ifndef __cplusplus
#  define true 1
#  define false 0

#  define bool _Bool

#  define __bool_true_false_are_defined 1
#endif


typedef unsigned long int  size_t;
typedef signed long int    ssize_t;
typedef signed long int    ptrdiff_t;
typedef signed long int    intptr_t;


typedef unsigned char*     ptr_t;
typedef float              float32;
typedef double             float64;
typedef signed char        int8;
typedef unsigned char      uint8;
typedef uint8              byte_t;
typedef signed short int   int16;
typedef unsigned short int uint16;
typedef signed int         int32;
typedef unsigned int       uint32;
typedef signed long int    int64;
typedef unsigned long int  uint64;

typedef uint8  bool8;
typedef uint16 bool16;
typedef uint32 bool32;

typedef ptr_t  Ptr;
typedef uint8  Byte;

typedef float  F32;
typedef double F64;

typedef int8   I8;
typedef uint8  U8;
typedef int16  I16;
typedef uint16 U16;
typedef int32  I32;
typedef uint32 U32;
typedef int64  I64;
typedef uint64 U64;


/* LIMITS rappresentations of types.
   The suffix to add in order to get the correct type
   may be dependent on compiler implementation.
   We're using `static_asserts` in `types.c` in order to make
   sure that we get the correct size for the suffices that we use.
   If we want to support more compiler/architectures, those literals
   should be wrapped with a `#if` preprocessor macros */
#define I32_LIT(x) ((I32) (x))             // Example: 1234
#define U32_LIT(x) ((U32) (CONCAT(x, U)))  // Example: 1234U
#define I64_LIT(x) ((I64) (CONCAT(x, L)))  // Example: 1234L
#define U64_LIT(x) ((U64) (CONCAT(x, UL))) // Example: 1234UL



#define U8_MAX  ( U8_LIT(255))
#define U16_MAX (U16_LIT(65535))
#define U32_MAX (U32_LIT(4294967295))
#define U64_MAX (U64_LIT(18446744073709551615))

#define I8_MAX  ( I8_LIT(127))
#define I16_MAX (I16_LIT(32767))
#define I32_MAX (I32_LIT(2147483647))
#define I64_MAX (I64_LIT(9223372036854775807))

#define I8_MIN  ( I8_LIT(-128))
#define I16_MIN (I16_LIT(-32768))
#define I32_MIN (I32_LIT(-2147483648))
#define I64_MIN (I64_LIT(-9223372036854775808))




/* Very simple c-string wrappers with len precomputed, and a buffer size associated.
   The data points to a valid c-string and it is _GUARANTEED_ to be NULL-TERMINATED.
   
   This string can store up to 2 GigaBytes of content. */

typedef struct Str32
{
    I32   bufsize;
    I32   len;
    char *data;
} Str32;

#define STR32_LIT(S)                            \
    ((Str32) {                                  \
        (I32) ARRAY_LEN(S),                     \
            (I32) STRLIT_LEN(S),                \
            (char*) S                           \
            })


#define cstr_to_str32(S) __cstr_to_str32__(S, strlen(S))
static inline Str32
__cstr_to_str32__(char *s, size_t str_len)
{
    I32 clamped_str_len = (I32) (str_len & I32_MAX);
    Str32 result = { clamped_str_len, clamped_str_len, s};
    return result;
}



#if __PAL_LINUX__
#  if __PAL_ARCHITECTURE_SIZE__ == 64
typedef long int time_t;
#  else
#    error "Double check what's the correct time_t rappresentation for 32 bit machines."
#  endif
#endif

struct tm
{
    int tm_sec;                 /* Seconds.	[0-60] (1 leap second) */
    int tm_min;                 /* Minutes.	[0-59] */
    int tm_hour;                /* Hours.	[0-23] */
    int tm_mday;                /* Day.		[1-31] */
    int tm_mon;                 /* Month.	[0-11] */
    int tm_year;                /* Year	- 1900.  */
    int tm_wday;                /* Day of week.	[0-6] */
    int tm_yday;                /* Days in year.[0-365]	*/
    int tm_isdst;               /* DST.		[-1/0/1]*/

#if	 __GNUC__
    /* GNU C Extension only */
    long int    tm_gmtoff;	  /* Seconds east of UTC.  */
    const char *tm_zone;        /* Timezone abbreviation.  */
# else
    long int    ___padding1___;  /* Seconds east of UTC.  */
    const char *___padding2___;  /* Timezone abbreviation.  */
# endif
};



__END_DECLS

#endif
