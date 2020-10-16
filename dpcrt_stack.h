#pragma once


#include "dpcrt_utils.h"
#include "dpcrt_allocators.h"
#include <stddef.h>
#include <stdint.h>

typedef struct stack {
    void *elems;
    miface_t *allocator;
    uint32_t cnt;
    uint32_t elem_size;
} stack_t;

// Marker to emphasize what the stack contains
// Example: STACK(int) my_stack;
//    used to emphasize that the stack contains ingegers
#define STACK(type) stack_t

#define SPUSH(s) (_stack_push(s))
#define SPOP(s) (_stack_pop(s))
#define SGET(s, idx) (_stack_get((s), idx))
#define SCNT(s) ((s)->cnt)
#define SEMPTY(s) ((s)->cnt == 0)
#define SPEEK(s) (_stack_get(s, SCNT(s) - 1))
#define SLAST(s) (_stack_get(s, 0))
#define SCLEAR(s) (_stack_clear(s))

#define SMAKE(elem_type, allocator, cnt) (_stack_make(sizeof(elem_type), allocator, cnt))
#define SDUP(s) (_stack_dup(s, s->allocator))
#define SDUPA(s, allocator) (_stack_dup(s, allocator))

#define STACK_DECL_TYPE(sdecl_name, selem_type, selem_name)                                                                                                    \
    typedef struct {                                                                                                                                           \
        union {                                                                                                                                                \
            stack_t stack;                                                                                                                                     \
            struct {                                                                                                                                           \
                selem_type *selem_name;                                                                                                                        \
            };                                                                                                                                                 \
        };                                                                                                                                                     \
    } sdecl_name;

static inline void *_stack_push(stack_t *s)
{
    void *newaddr = MRENEW(s->allocator, s->elems, (s->cnt + 1) * s->elem_size);
    if (newaddr) {
        s->elems = newaddr;
        void *result = newaddr + (s->cnt) * s->elem_size;
        s->cnt++;
        return result;
    }
    return NULL;
}

static inline bool _stack_pop(stack_t *s)
{
    if (s->cnt > 0) {
        void *newElems = MRENEW(s->allocator, s->elems, s->elem_size * (s->cnt - 1));
        if (newElems) {
            s->elems = newElems;
            s->cnt--;
        }
        return true;
    }
    return false;
}

static inline void *_stack_get(stack_t *s, uint32_t idx)
{
    if (idx >= s->cnt)
        return NULL;
    return (char *)s->elems + (s->elem_size) * idx;
}

static inline void _stack_clear(stack_t *s)
{
    if (s->allocator && s->elems)
        MDEL(s->allocator, s->elems);
    s->elems = NULL;
    s->cnt = 0;
}

static inline stack_t _stack_make(size_t elem_size, miface_t *allocator, size_t cnt)
{
    stack_t result = { 0 };
    result.allocator = allocator;
    if (cnt > 0) {
        result.elems = MNEW(allocator, elem_size * cnt);
        if (result.elems)
            result.cnt = cnt;
    }
    result.elem_size = elem_size;
    return result;
}
static inline stack_t _stack_dup(stack_t *s, miface_t *allocator)
{
    stack_t result = { 0 };
    result.allocator = allocator;
    result.elems = MNEW(allocator, SCNT(s) * s->elem_size);
    if (result.elems) {
        result.cnt = SCNT(s);
        result.elem_size = s->elem_size;
        memcpy(result.elems, s->elems, SCNT(s) * s->elem_size);
    }
    return result;
}
