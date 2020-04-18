#ifndef HGUARD_d5c911ad5a844c18b81fe434e9fe5f42
#define HGUARD_d5c911ad5a844c18b81fe434e9fe5f42

#include "dpcrt_types.h"
#include "dpcrt_utils.h"
#include "dpcrt_mem.h"

__BEGIN_DECLS


typedef struct BufHdr {
    size_t len;
    size_t cap;
    char buf[];
} BufHdr;


/*
This macro could be used as a way to mark the declaration of a variable to make
the intententions more clear
     BUF(float) p = NULL; // The information that `p` will be used as a stretchy buffer is readily available
     ...
     BUF_PUSH(p, 10.0)

     // Or even from functions that expects a stretchy buffer instead of a trivial C pointer

     void f(BUF(float) p);
     
*/
#define BUF(type)  type*

#define BUF__HDR(b) ((BufHdr *)((char *)(b) - offsetof(BufHdr, buf)))

#define BUF_LEN(b) ((b) ? BUF__HDR(b)->len : 0)
#define BUF_CAP(b) ((b) ? BUF__HDR(b)->cap : 0)
#define BUF_END(b) ((b) + BUF_LEN(b))
#define BUF_SIZEOF(b) ((b) ? BUF_LEN(b)*sizeof(*b) : 0)

#define BUF_FREE(b) ((b) ? (free(BUF__HDR(b)), (b) = NULL) : 0)
#define BUF_FIT(b, n) ((n) <= BUF_CAP(b) ? 0 : ((b) = buf__grow((b), (n), sizeof(*(b)))))
#define BUF_PUSH(b, ...) (BUF_FIT((b), 1 + BUF_LEN(b)), (b)[BUF__HDR(b)->len++] = (__VA_ARGS__))
#define BUF_POP(b, ...) ((b)[BUF__HDR(b)->len--])
#define BUF_PRINTF(b, ...) ((b) = buf__printf((b), __VA_ARGS__))
#define BUF_CLEAR(b) ((b) ? BUF__HDR(b)->len = 0 : 0)
 
#define BUF_NEW(TYPE, CNT) (((TYPE)*) __buf_new__((CNT), sizeof(TYPE)))
void *__buf_new__(size_t cnt, size_t elem_size)
{
    void *b = NULL;
    ((cnt) <= BUF_CAP(b) ? 0 : ((b) = buf__grow((b), (n), elem_size)));
    return b;
}

static void *buf__grow(const void *buf, size_t new_len, size_t elem_size)
{
    assert(BUF_CAP(buf) <= (SIZE_MAX - 1)/2);
    size_t new_cap = CLAMP_MIN(2*BUF_CAP(buf), MAX(new_len, 16));
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf))/elem_size);
    size_t new_size = offsetof(BufHdr, buf) + new_cap*elem_size;
    BufHdr *new_hdr;
    if (buf) {
        new_hdr = xrealloc(BUF__HDR(buf), new_size);
    } else {
        new_hdr = xmalloc(new_size);
        new_hdr->len = 0;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

static char *buf__printf(char *buf, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t cap = BUF_CAP(buf) - BUF_LEN(buf);
    size_t n = 1 + vsnprintf(BUF_END(buf), cap, fmt, args);
    va_end(args);
    if (n > cap) {
        BUF_FIT(buf, n + BUF_LEN(buf));
        va_start(args, fmt);
        size_t new_cap = BUF_CAP(buf) - BUF_LEN(buf);
        n = 1 + vsnprintf(BUF_END(buf), new_cap, fmt, args);
        assert(n <= new_cap);
        va_end(args);
    }
    BUF__HDR(buf)->len += n - 1;
    return buf;
}


typedef BUF(char) STR;

#define STR_SIZE(b)   ((BUF_LEN(s)))
#define STR_LEN(s)    ((BUF_LEN(s)) - 1) /* Do not count the null terminator */
#define STR_CAP(s)    (BUF_CAP(s))
#define STR_END(s)    (BUF_END(s))

#define STR_FREE(s)   (BUF_FREE(s))
#define STR_FIT(s, n) (((n) + 1) <= STR_CAP(b) ? 0 : ((s) = str__grow((s), (n + 1))))
#define STR_PUSH(s, ...) (STR_FIT((s), 1 + STR_LEN(s)), (s)[BUF__HDR(s)->len] = (__VA_ARGS__), (s)[++(BUF__HDR(s)->len)] = 0)
#define STR_POP(s, ...) ((s)[BUF__HDR(b)->len - 1] ? (s)[BUF__HDR(b)->len--] : '\0')
#define STR_PRINTF(s, ...) ((s) = buf__printf((s), __VA_ARGS__))
#define STR_CLEAR(s) ((s) ? (BUF__HDR(s)->len = 1, s[0] = '\0') : 0)

static void *str__grow(const void *buf, size_t new_len)
{
    assert(BUF_CAP(buf) <= (SIZE_MAX - 1)/2);
    size_t new_cap = CLAMP_MIN(2*BUF_CAP(buf), MAX(new_len, 16));
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf)));
    size_t new_size = offsetof(BufHdr, buf) + new_cap;
    BufHdr *new_hdr;
    if (buf) {
        new_hdr = xrealloc(BUF__HDR(buf), new_size);
    } else {
        new_hdr = xmalloc(new_size);
        *((char * )new_hdr) = '\0';
        new_hdr->len = 1;
    }
    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

__END_DECLS

#endif /* HGUARD_d5c911ad5a844c18b81fe434e9fe5f42 */
