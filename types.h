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

#ifndef HGUARD_ac8c7d54818e489096b120ef3afb4a25
#define HGUARD_ac8c7d54818e489096b120ef3afb4a25

#include "compiler.h"


__BEGIN_DECLS

/* Bool type */
#ifndef __cplusplus
#  define true 1
#  define false 0

#  define bool _Bool

#  define __bool_true_false_are_defined 1
#endif
/* ------------------ */



/* Standard generic types */
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
/* ----------------------- */


/* LIMITS rappresentations of types.
   The suffix to add in order to get the correct type
   may be dependent on compiler implementation.
   We're using `static_asserts` in `types.c` in order to make
   sure that we get the correct size for the suffices that we use.
   If we want to support more compiler/architectures, those literals
   should be wrapped with a `#if` preprocessor macros */
#if __PAL_ARCHITECTURE_SIZE__ == 64
#  define  I8_LIT(x) ( (I8) (x))
#  define  U8_LIT(x) ( (U8) (x))
#  define I16_LIT(x) ((I16) (x))
#  define U16_LIT(x) ((U16) (x))
#  define I32_LIT(x) ((I32) (x))             // Example: 1234
#  define U32_LIT(x) ((U32) (CONCAT(x, U)))  // Example: 1234U
#  define I64_LIT(x) ((I64) (CONCAT(x, L)))  // Example: 1234L
#  define U64_LIT(x) ((U64) (CONCAT(x, UL))) // Example: 1234UL
#elif  __PAL_ARCHITECTURE_SIZE__ == 32
#  define  I8_LIT(x) ( (I8) (x))
#  define  U8_LIT(x) ( (U8) (x))
#  define I16_LIT(x) ((I16) (x))
#  define U16_LIT(x) ((U16) (x))
#  define I32_LIT(x) ((I32) (x))             // Example: 1234
#  define U32_LIT(x) ((U32) (CONCAT(x, U)))  // Example: 1234U
#  define I64_LIT(x) ((I64) (CONCAT(x, LL)))  // Example: 1234LL
#  define U64_LIT(x) ((U64) (CONCAT(x, ULL))) // Example: 1234ULL
#endif


/* Tecnically the C99 Standard defines a Hexadecimal Literal
   to be the type of the smallest possible type that can contain it.
   For example `0x80` is a uint8 type, while `0xfff` is a uint16 type.

   Tecnically for those macros there's no need to wrap the constants
   with a `<XX>_LIT` macro; but we want to remain conservative in the
   case the compiler is not fully C99 STANDARD compliant.
*/
#define U8_MAX  ( U8_LIT(0xff))
#define U16_MAX (U16_LIT(0xffff))
#define U32_MAX (U32_LIT(0xfffffffff))
#define U64_MAX (U64_LIT(0xffffffffffffffff))

#define U8_MIN  ( U8_LIT(0x00))
#define U16_MIN (U16_LIT(0x0000))
#define U32_MIN (U32_LIT(0x00000000))
#define U64_MIN (U64_LIT(0x0000000000000000))


#define I8_MAX  ( I8_LIT(0x7f))
#define I16_MAX (I16_LIT(0x7fff))
#define I32_MAX (I32_LIT(0x7fffffff))
#define I64_MAX (I64_LIT(0x7fffffffffffffff))

#define I8_MIN  ( I8_LIT(0x80))
#define I16_MIN (I16_LIT(0x8000))
#define I32_MIN (I32_LIT(0x80000000))
#define I64_MIN (I64_LIT(0x8000000000000000))




/* Very simple c-string wrappers with len precomputed, and a buffer size associated.
   The data points to a valid c-string and it is _GUARANTEED_ to be NULL-TERMINATED.

   This string can store up to 2 GigaBytes of content. */

typedef struct Str32
{
    I32   bufsize;
    I32   len;
    char *data;
} Str32;

/* PStr32 :: stands for `PackedStr32`
   In the packed version of `Str32`, the string payload follows immediately
   the header (eg it does not live in a seperate place in memory) */
typedef struct PStr32
{
    I32 bufsize;
    I32 len;
    char data[];
} PStr32;

#define STR32_LIT(S)                            \
    ((Str32) {                                  \
        (I32) ARRAY_LEN(S),                     \
            (I32) STRLIT_LEN(S),                \
            (char*) S                           \
            })

#define PSTR32_LIT(S) \
    ((PStr32) {                                  \
        (I32) ARRAY_LEN(S),                      \
            (I32) STRLIT_LEN(S),                 \
            S                                    \
            })


#define cstr_to_str32(S) __cstr_to_str32__(S, strlen(S))
static inline Str32
__cstr_to_str32__(char *s, size_t str_len)
{
    I32 clamped_str_len = (I32) (str_len & (size_t) I32_MAX);
    Str32 result = { clamped_str_len, clamped_str_len, s };
    return result;
}



#if __PAL_LINUX__
#  if __PAL_ARCHITECTURE_SIZE__ == 64
typedef long int time_t;
#  else
#    error "Double check what's the correct time_t rappresentation for 32 bit machines."
#  endif
#endif


__END_DECLS

#endif  /* HGUARD_ac8c7d54818e489096b120ef3afb4a25 */
