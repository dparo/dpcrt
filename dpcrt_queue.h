#pragma once

#include "dpcrt_types.h"
#include "dpcrt_utils.h"
#include <stdint.h>

// Circular Queue defines
// Example struct
//
// typedef struct my_circular_queue
//     int32_t blen;            // Buffer Length eg Max length
//     int32_t s;               // Start index
//     int32_t len;             // len of the queue: [0...buffer_len - 1]
//     int32_t *buf;            // The data to use
// } my_circular_queue_t;
#include "utils.h"


#define CQMAKE(q, L) do { (q)->s = 0; (q)->blen = 0; (q)->len = 0; (q)->buf = NULL; _cqmake(L, sizeof((q)->buf[0]), (void **) &(q)->buf, &(q)->blen); } while(0)
#define CQIDX(q, i)  _cqidx((q)->s, (q)->blen, i)
#define CQREF(q, i)  ((q)->len == 0 ? NULL : (&(q)->buf[CQIDX(q, i)]))
#define CQGET(q, i)  (*CQREF(q, i))

#define CQREF_FIRST(q)  (&(q)->buf[CQIDX(q, 0)])
#define CQREF_LAST(q)  (&(q)->buf[CQIDX(q, (q)->len - 1)])

#define CQCLEAR(q) ((q)->len = 0)
#define CQEMPTY(q) ((q)->len == 0)

#define CQENQUEUE_N(q, N)                            \
    do {                                             \
        int32_t m = MIN((q)->len + N, (q)->blen);    \
        (q)->len = m;                                \
    } while(0)

#define CQENQUEUE(q, e)  \
    do { \
        if ((q)->len == (q)->blen) {               \
            (q)->buf[CQIDX(q, 0)] = (e);           \
            (q)->s = CQIDX(q, 1);                  \
        } else {                                   \
            (q)->buf[CQIDX(q, (q)->len)] = (e);    \
            (q)->len++;                            \
        } \
    } while(0)

#define CQDEQUEUE_N(q, N)                  \
    do {                                   \
        if (!CQEMPTY(q)) {                 \
            int32_t m = MIN((q)->len, N);  \
            (q)->s = CQIDX(q, m);          \
            (q)->len -= m;                 \
         }                                 \
    } while(0)

#define CQDEQUEUE(q) (q)->len == 0 ? false : ((q)->s = CQIDX(q, 1), (q)->len -= 1, true)






inline static  __attribute__ ((pure)) int32_t _cqidx(int32_t qstart, int32_t qblen, int32_t i)
{
    return (qstart + i) % qblen;
}

inline static void _cqmake(int32_t num_elems, int32_t elem_size, void **ptr, int32_t *blen)
{
    *ptr = calloc(num_elems, elem_size);
    if (*ptr) {
        *blen = num_elems;
    }
}
