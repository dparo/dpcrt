#ifndef STRING_H
#define STRING_H

#include "utils.h"

__BEGIN_DECLS

extern void *
std_func_memcpy (void *__restrict dest, const void *__restrict src, size_t n)
    ATTRIB_NOTHROW ATTRIB_NONNULL(1, 2);

extern void *
std_func_memmove (void *dest, const void *src, size_t n)
    ATTRIB_NOTHROW ATTRIB_NONNULL(1, 2);

extern void *
std_func_memset (void *s, int c, size_t n)
    ATTRIB_NOTHROW ATTRIB_NONNULL(1);

extern int
std_func_memcmp (const void *s1, const void *s2, size_t n)
    ATTRIB_NOTHROW ATTRIB_PURE ATTRIB_NONNULL(1, 2);

extern char *
std_func_strcpy (char *__restrict dest, const char *__restrict src)
    ATTRIB_NOTHROW ATTRIB_NONNULL(1, 2);

extern int
std_func_strcmp (const char *s1, const char *s2)
    ATTRIB_NOTHROW ATTRIB_NONNULL(1, 2);

extern int
std_func_strncmp (const char *s1, const char *s2, size_t n)
    ATTRIB_NOTHROW ATTRIB_PURE ATTRIB_NONNULL(1, 2);


extern size_t
std_func_strlen (const char *__s)
    ATTRIB_NOTHROW ATTRIB_PURE ATTRIB_NONNULL(1);


#if __GNUC__
#  define COMPILER_BUILTIN_memcpy  __builtin_memcpy
#  define COMPILER_BUILTIN_memmove __builtin_memmove
#  define COMPILER_BUILTIN_memset  __builtin_memset
#  define COMPILER_BUILTIN_memcmp  __builtin_memcmp
#  define COMPILER_BUILTIN_strcpy  __builtin_strcpy
#  define COMPILER_BUILTIN_strcmp  __builtin_strcmp
#  define COMPILER_BUILTIN_strncmp __builtin_strncmp
#  define COMPILER_BUILTIN_strlen  __builtin_strlen
#else
#  define COMPILER_BUILTIN_memcpy    std_func_memcpy
#  define COMPILER_BUILTIN_memmove   std_func_memmove
#  define COMPILER_BUILTIN_memset    std_func_memset
#  define COMPILER_BUILTIN_memcmp    std_func_memcmp
#  define COMPILER_BUILTIN_strcpy    std_func_strcpy
#  define COMPILER_BUILTIN_strcmp    std_func_strcmp
#  define COMPILER_BUILTIN_strncmp   std_func_strncmp
#  define COMPILER_BUILTIN_strlen    std_func_strlen
#endif

#define memcpy    COMPILER_BUILTIN_memcpy
#define memcpy    COMPILER_BUILTIN_memcpy
#define memmove   COMPILER_BUILTIN_memmove
#define memset    COMPILER_BUILTIN_memset
#define memcmp    COMPILER_BUILTIN_memcmp
#define strcpy    COMPILER_BUILTIN_strcpy
#define strcmp    COMPILER_BUILTIN_strcmp
#define strncmp   COMPILER_BUILTIN_strncmp
#define strlen    COMPILER_BUILTIN_strlen


__END_DECLS

#endif
