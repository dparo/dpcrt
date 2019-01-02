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
#ifndef HGUARD_c4340e2c7dcd4d22868577bdf05a9ca0
#define HGUARD_c4340e2c7dcd4d22868577bdf05a9ca0

#include "dpcrt_utils.h"

__BEGIN_DECLS

extern void *
libc_memcpy (void *__restrict dest, const void *__restrict src, size_t n)
    ATTRIB_NOTHROW ATTRIB_NONNULL(1, 2);

extern void *
libc_memmove (void *dest, const void *src, size_t n)
    ATTRIB_NOTHROW ATTRIB_NONNULL(1, 2);

extern void *
libc_memset (void *s, int c, size_t n)
    ATTRIB_NOTHROW ATTRIB_NONNULL(1);

extern int
libc_memcmp (const void *s1, const void *s2, size_t n)
    ATTRIB_NOTHROW ATTRIB_PURE ATTRIB_NONNULL(1, 2);

extern char *
libc_strcpy (char *__restrict dest, const char *__restrict src)
    ATTRIB_NOTHROW ATTRIB_NONNULL(1, 2);

extern int
libc_strcmp (const char *s1, const char *s2)
    ATTRIB_NOTHROW ATTRIB_NONNULL(1, 2);

extern int
libc_strncmp (const char *s1, const char *s2, size_t n)
    ATTRIB_NOTHROW ATTRIB_PURE ATTRIB_NONNULL(1, 2);


extern size_t
libc_strlen (const char *s)
    ATTRIB_NOTHROW ATTRIB_PURE ATTRIB_NONNULL(1);


char *strerror (int err_number) ATTRIB_NOTHROW;

#if __GNUC__ || __clang__
#  define RESOLVE_memcpy  __builtin_memcpy
#  define RESOLVE_memmove __builtin_memmove
#  define RESOLVE_memset  __builtin_memset
#  define RESOLVE_memcmp  __builtin_memcmp
#  define RESOLVE_strcpy  __builtin_strcpy
#  define RESOLVE_strcmp  __builtin_strcmp
#  define RESOLVE_strncmp __builtin_strncmp
#  define RESOLVE_strlen  __builtin_strlen
#else
#  define RESOLVE_memcpy    libc_memcpy
#  define RESOLVE_memmove   libc_memmove
#  define RESOLVE_memset    libc_memset
#  define RESOLVE_memcmp    libc_memcmp
#  define RESOLVE_strcpy    libc_strcpy
#  define RESOLVE_strcmp    libc_strcmp
#  define RESOLVE_strncmp   libc_strncmp
#  define RESOLVE_strlen    libc_strlen
#endif

#define memcpy    RESOLVE_memcpy
#define memmove   RESOLVE_memmove
#define memset    RESOLVE_memset
#define memcmp    RESOLVE_memcmp
#define strcpy    RESOLVE_strcpy
#define strcmp    RESOLVE_strcmp
#define strncmp   RESOLVE_strncmp
#define strlen    RESOLVE_strlen


__END_DECLS

#endif  /* HGUARD_c4340e2c7dcd4d22868577bdf05a9ca0 */
