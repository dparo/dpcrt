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
#include "dpcrt_compiler.h"


__BEGIN_DECLS

#ifndef NULL
#  define NULL ((void*) 0x0)
#endif


/* Bool type */
#ifndef __cplusplus
#  define true  1
#  define false 0

#if __STDC_VERSION__ >= STD_C99_VERSION
#  define bool _Bool
#else
typedef int bool;
#endif

#  define __bool_true_false_are_defined 1
#endif
/* ------------------ */



/* Standard generic types */
typedef unsigned long int  size_t;
typedef signed long int    ssize_t;
typedef signed long int    ptrdiff_t;
typedef signed long int    intptr_t;
typedef unsigned long int  uintptr_t;

/* The C-Standard guarantess that any pointer
   can be safely casted into an integer type
   by resorting back to the C standard types `inptr_t`
   and `uintptr_t`.

   Even though on modern machines one tipically uses a `size_t`
   to rappresent a pointer as an integer, this is not techically
   true for every machine. So if you actually care
   about being politically correct here cast the pointers to `usize`
   and not to `size_t`.

   `size_t` types are guaranteed from the C-standard to be able to
   rappresent any possible indexing within the machine (64 bit typical) */
typedef intptr_t   isize_t;
typedef uintptr_t  usize_t;
typedef isize_t    isize;
typedef usize_t    usize;

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

typedef uint8  BYTE;

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


typedef U8* PTR;

#if __GNUC__ || __clang__
typedef signed   __int128  int128_t;
typedef unsigned __int128  uint128_t;
#else
#  error "Check for this COMPILER and ARCHITECTURE (Can we even use 128 bits wide integers?"
#endif


typedef int128_t  int128;
typedef uint128_t uint128;
typedef int128_t  I128;
typedef uint128_t U128;


#if __DPCRT_LINUX
#  if __DPCRT_ARCH_SIZE == 64
typedef long time_t;
#  else
typedef long long time_t;
#  endif
#endif


/* ----------------------- */


/* LIMITS rappresentations of types.
   The suffix to add in order to get the correct type
   may be dependent on compiler implementation.
   We're using `static_asserts` in `types.c` in order to make
   sure that we get the correct size for the suffices that we use.
   If we want to support more compiler/architectures, those literals
   should be wrapped with a `#if` preprocessor macros */
#if __DPCRT_ARCH_SIZE == 64
#  define  I8_LIT(x) ( (I8) (x))
#  define  U8_LIT(x) ( (U8) (x))
#  define I16_LIT(x) ((I16) (x))
#  define U16_LIT(x) ((U16) (x))
#  define I32_LIT(x) ((I32) (x))
#  define U32_LIT(x) ((U32) x ## U)
#  define I64_LIT(x) ((I64) x ## L)
#  define U64_LIT(x) ((U64) x ## UL)
#elif __DPCRT_ARCH_SIZE == 32
#  define  I8_LIT(x) ( (I8) (x))
#  define  U8_LIT(x) ( (U8) (x))
#  define I16_LIT(x) ((I16) (x))
#  define U16_LIT(x) ((U16) (x))
#  define I32_LIT(x) ((I32) (x))
#  define U32_LIT(x) ((U32) x ## U)
#  define I64_LIT(x) ((I64) x ## LL)
#  define U64_LIT(x) ((U64) x ## ULL)
#endif

#define I8_C(x)  I8_LIT(x)
#define U8_C(x)  U8_LIT(x)
#define I16_C(x) I16_LIT(x)
#define U16_C(x) U16_LIT(x)
#define I32_C(x) I32_LIT(x)
#define U32_C(x) U32_LIT(x)
#define I64_C(x) I64_LIT(x)
#define U64_C(x) U64_LIT(x)


#ifndef INT8_C
#  define INT8_C(x) I8_C(x)
#endif
#ifndef UINT8_C
#  define UINT8_C(x) U8_C(x)
#endif
#ifndef INT16_C
#  define INT16_C(x) I16_C(x)
#endif
#ifndef UINT16_C
#  define UINT16_C(x) U16_C(x)
#endif
#ifndef INT32_C
#  define INT32_C(x) I32_C(x)
#endif
#ifndef UINT32_C
#  define UINT32_C(x) U32_C(x)
#endif
#ifndef INT64_C
#  define INT64_C(x) I64_C(x)
#endif
#ifndef UINT64_C
#  define UINT64_C(x) U64_C(x)
#endif





/* Tecnically the C99 Standard defines a Hexadecimal Literal
   to be the type of the smallest possible type that can contain it.
   For example `0x80` is a uint8 type, while `0xfff` is a uint16 type.

   Tecnically for those macros there's no need to wrap the constants
   with a `<XX>_LIT` macro; but we want to remain conservative in the
   case the compiler is not fully C99 STANDARD compliant.
*/
#define U8_MAX  (U8_LIT(0xff))
#define U16_MAX (U16_LIT(0xffff))
#define U32_MAX (U32_LIT(0xfffffffff))
#define U64_MAX (U64_LIT(0xffffffffffffffff))

#define U8_MIN  (U8_LIT(0x00))
#define U16_MIN (U16_LIT(0x0000))
#define U32_MIN (U32_LIT(0x00000000))
#define U64_MIN (U64_LIT(0x0000000000000000))


#define I8_MAX  (I8_LIT(0x7f))
#define I16_MAX (I16_LIT(0x7fff))
#define I32_MAX (I32_LIT(0x7fffffff))
#define I64_MAX (I64_LIT(0x7fffffffffffffff))

#define I8_MIN  (I8_LIT(0x80))
#define I16_MIN (I16_LIT(0x8000))
#define I32_MIN (I32_LIT(0x80000000))
#define I64_MIN (I64_LIT(0x8000000000000000))

#if __DPCRT_ARCH_SIZE == 64
#  define SIZE_MAX U64_MAX
#elif __DPCRT_ARCH_SIZE == 32
#  define SIZE_MAX U32_MAX
#endif

typedef struct range {
    I32 start;
    I32 end;
} range_t;

typedef struct wrange
{
    I64 start;
    I64 end;
} wrange_t;

typedef struct prange
{
    void      *base;
    uintptr_t  end;
} prange_t;


#define RANGE(start, end)  ((struct range)  {(start), (end)})
#define WRANGE(start, end) ((struct wrange) {(start), (end)})
#define PRANGE(base, end)  ((struct prange) {(base), (end)})




// str32_t are valid c strings by definition. They are null terminated
typedef struct str32 {
    char *cstr;
    int32_t len;
} str32_t;

typedef struct str32_list {
    str32_t *ss;
    int32_t cnt;
} str32_list_t;

#define STR32(s) ((str32_t){ s, strlen(s) })

// Substring usually live inside a c string or another str32_t. And
// thus they are not guaranteed to be null terminated
typedef struct substr {
    char *base;
    int32_t len;
} substr32_t;

typedef struct path {
    str32_t drive;
    str32_t dirpath;
    str32_t fileroot;
    str32_t fileext;
} path_t;




static inline bool streq(const char *s1, const char *s2)
{
    return 0 == strcmp(s1, s2);
}

static inline int32_t str32_cmp(const str32_t s1, const str32_t s2)
{
    int32_t diff = s1.len - s2.len;
    if (diff != 0)
        return diff;
    else
        return strcmp(s1.cstr, s2.cstr);
}

static inline int32_t str32_eq(const str32_t s1, const str32_t s2)
{
    return 0 == str32_cmp(s1, s2);
}

static inline int32_t str32_find(const str32_t s1, const str32_t s2)
{
    char *ptr = strstr(s1.cstr, s2.cstr);
    if (ptr == NULL) {
        return INT32_MIN;
    } else {
        return ptr - s1.cstr;
    }
}

static inline bool str32_contains(const str32_t s1, const str32_t s2)
{
    return str32_find(s1, s2) >= 0;
}

static inline int32_t substr_cmp(const substr32_t s1, const substr32_t s2)
{
    int32_t diff = s1.len - s2.len;
    if (diff != 0)
        return diff;
    else
        return strncmp(s1.base, s2.base, s1.len);
}

static inline int32_t substr_eq(const substr32_t s1, const substr32_t s2)
{
    return 0 == substr_cmp(s1, s2);
}

static inline str32_t str32_dup(miface_t *alloc, str32_t s)
{
    str32_t result = { 0 };
    char *mem = MNEW(alloc, s.len + 1);
    if (mem) {
        strncpy(mem, s.cstr, s.len + 1);
        result.cstr = mem;
        result.len = s.len;
    }
    return result;
}

static inline str32_t str32_fmt(miface_t *alloc, char *fmt, ...)
{
    str32_t result = { 0 };
    va_list ap;
    va_start(ap, fmt);
    int required_len = vsnprintf(NULL, 0, fmt, ap);
    result.cstr = MNEW(alloc, required_len + 1);
    va_start(ap, fmt);
    if (result.cstr) {
        result.cstr[0] = '\0';
        result.cstr[required_len] = '\0';
        result.len = required_len;
        vsprintf(result.cstr, fmt, ap);
    }

    va_end(ap);
    return result;
}

static inline str32_list_t str32_split(miface_t *alloc, str32_t s, char c)
{
    str32_list_t result = { 0 };
    int32_t start = 0;
    int32_t i = 0;
    for (i = 0; i < s.len; i++) {
        if (s.cstr[i] == c) {
            result.ss = MRENEW(alloc, result.ss, (result.cnt + 1) * sizeof(str32_t));
            if (!result.ss) {
                str32_list_t empty_list = { 0 };
                return empty_list;
            }
            result.ss[result.cnt++] = str32_fmt(alloc, "%.*s", i - start, &(s.cstr[start]));
            start = i + 1;
        }
    }

    result.ss = MRENEW(alloc, result.ss, (result.cnt + 1) * sizeof(str32_t));
    if (!result.ss) {
        str32_list_t empty_list = { 0 };
        return empty_list;
    }
    result.ss[result.cnt++] = str32_fmt(alloc, "%.*s", i - start, &(s.cstr[start]));
    return result;
}

static inline int32_t str32_list_find(str32_list_t *l, str32_t s)
{
    for (int32_t i = 0; i < l->cnt; i++) {
        if (str32_eq(l->ss[i], s)) {
            return i;
        }
    }
    return INT32_MIN;
}

static inline bool str32_list_contains(str32_list_t *l, str32_t s)
{
    return str32_list_find(l, s) >= 0;
}



static inline bool _is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

static inline bool _is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline bool _is_alnum(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline bool _is_space(char c)
{
    return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v');
}

static inline char *substr32_to_cstr(miface_t *allocator, substr32_t *s)
{
    char *result = NULL;
    result = MNEW(allocator, s->len + 1);
    if (result) {
        memcpy(result, s->base, s->len);
        result[s->len] = 0;
    }
    return result;
}




bool cstr_to_int32(char *string, int32_t *i);
bool cstr_to_double(char *string, double *d);





typedef struct Str32Hdr {
    I32   len;
} Str32Hdr;

/* Very simple c-string wrappers with len precomputed, and a buffer size associated.
   The data points to a valid c-string and it is _GUARANTEED_ to be NULL-TERMINATED.

   This string can store up to 2 GigaBytes of content. */
typedef struct Str32 {
    I32   len;
    char *data;
} Str32;

/* PStr32 :: stands for `PackedStr32`
   In the packed version of `Str32`, the string payload follows immediately
   the header (eg it does not live in a seperate place in memory) */
typedef struct PStr32
{
    I32 len;
    char data[];
} PStr32;

#define STR32_LIT(S)                            \
    ((Str32) {                                  \
            (I32) STRLIT_LEN(S),                \
                (char*) S                       \
            })

#define PSTR32_LIT(S) \
    ((PStr32) {                                  \
            (I32) STRLIT_LEN(S),                 \
                S                                \
            })


#define cstr_to_str32(S) __cstr_to_str32__(S, strlen(S))
static inline Str32
__cstr_to_str32__(char *s, size_t str_len)
{
    Str32 result = {
        (I32) (str_len & (size_t) I32_MAX),
        s
    };
    return result;
}



/* /\* Singly Linked List *\/ */
/* #define SLIST_INIT(head)                        \ */
/*     (head)->next = NULL */

/* #define SLIST_PUSH(head, new_elem)              \ */
/*     do {                                        \ */
/*         (new_elem)->next = (head);              \ */
/*         (head) = (new_elem);                    \ */
/*     } while(0); */

/* #define SLIST_POP(head)              \ */
/*     do {                             \ */
/*         (head) = (head)->next;       \ */
/*     } while(0); */


/* #define SLIST_CHAIN(head, target, new_elem)     \ */
/*     do {                                        \ */
/*         assert(head);                           \ */
/*         (new_elem)->next = (target)->next;      \ */
/*         (target)->next = new_elem;              \ */
/*     } while(0) */

/* #define SLIST_UNCHAIN(head, target, ll_prev_in_chain)                   \ */
/*     do {                                                                \ */
/*         if ((head) == (target)) { SLIST_POP(head); }                    \ */
/*         else if ((ll_prev_in_chain)->next) { (ll_prev_in_chain)->next = (target)->next; } \ */
/*         (target)->next = NULL;                                          \ */
/*     } while(0); */



/* Doubly Linked List */
/* void DLIST_INIT(T *head) */
#define DLIST_HEAD_INIT(head)                   \
    do {                                        \
        (head)->next = NULL;                    \
        (head)->prev = NULL;                    \
    } while(0)

/* void DLIST_PUSH(T& *head, T *new_elem) */
#define DLIST_HEAD_PUSH(head, new_elem)                \
    do {                                               \
        (new_elem)->next = *(head);                    \
        (new_elem)->prev = NULL;                       \
        if (*(head)) { (*(head))->prev = new_elem; }   \
        *(head) = new_elem;                            \
    } while(0);


ATTRIB_CONST static inline void *
__dlist_head_pop__(void *restrict *restrict const head,
                   void *restrict *restrict const head_next,       // (*head)->next
                   void *restrict *restrict const head_next_prev)  // (*head)->next->prev)
{
    if (!(*head)) return NULL;
    void *const result = *head;
    void *const temp = *head_next;
    *head_next = NULL;
    *head = temp;
    if (*head_next)
        *(head_next_prev) = NULL;
    return result;
}

/* T* DLIST_POP(T& *head) */
#define DLIST_POP(head)                                                 \
    __dlist_pop__((void *restrict *restrict const) (head),              \
                  (void *restrict *restrict const) &((*(head))->next),  \
                  (void *restrict *restrict const) &((*(head))->next->prev))


/* void DLIST_ENQUEUE(T& *tail, T *new_elem) */
#define DLIST_ENQUEUE(tail, new_elem)           \
    do {                                        \
        (tail)->next = (new_elem);              \
        (new_elem)->next = NULL;                \
        (new_elem)->prev = (tail);              \
        (tail) = (new_elem);                    \
    } while(0);


ATTRIB_CONST static inline void *
__dlist_dequeue__(void *restrict *restrict const tail,
                  void *restrict *restrict const tail_prev,       // (*tail)->prev
                  void *restrict *restrict const tail_prev_next)  // (*tail)->prev->next)
{
    if (!(*tail)) return NULL;
    void *const result = *tail;
    void *const temp = *tail_prev;
    *tail_prev = NULL;
    *tail = temp;
    if (*tail_prev)
        *(tail_prev_next) = NULL;
    return result;
}

/* T* DLIST_DEQUEUE(T& *tail) */
#define DLIST_DEQUEUE(tail)                                             \
    __dlist_dequeue__((void *restrict *restrict const) &(tail),         \
                      (void *restrict *restrict const) &((*tail)->prev), \
                      (void *restrict *restrict const) &((*tail)->prev->next)))


/* void DLIST_CHAIN(T *head, T *target, T *new_elem) */
#define DLIST_CHAIN(head, target, new_elem)     \
    do {                                        \
        assert(head);                           \
        (new_elem)->prev = (target);            \
        (new_elem)->next = (target)->next;      \
        (target)->next   = &(new_elem);         \
    } while(0)

/* void DLIST_UNCHAIN(T *head, T *target) */
#define DLIST_UNCHAIN(head, target)                    \
    do {                                               \
        if (((target)->next))                          \
            ((target)->next)->prev = ((target)->prev); \
        if (((target)->prev))                          \
            ((target)->prev)->next = ((target)->next); \
        if ((target) == (*(head)))                     \
            (*(head)) = (target)->next;                \
        DLIST_INIT(target);                            \
    } while(0);


#define DLIST_FOREACH(type, it, head)                  \
    for (type it = head; it != NULL; it = it->next)

#define DLIST_FOREACH_ENQUEUED(type, it, tail)         \
    for (type it = tail; it != NULL; it = tail->prev)
