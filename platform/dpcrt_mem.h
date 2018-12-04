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

#ifndef HGUARD_77f71620e38a4ba09d5a55b0bc707a92
#define HGUARD_77f71620e38a4ba09d5a55b0bc707a92

#include "dpcrt_utils.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "dpcrt_types.h"
#include "dpcrt_pal.h"


__BEGIN_DECLS

enum alloc_strategy {
    AllocStrategy_Xmalloc       = 0,
    AllocStrategy_Malloc        = 1,
    AllocStrategy_Xcalloc       = 2,
    AllocStrategy_Calloc        = 3,
    AllocStrategy_AlignedAlloc  = 4,
    AllocStrategy_Mmap          = 5,
    AllocStrategy_Sbrk          = 6,          // @NOTE: Reallocation is supported only if it is the last allocated in a LIFO style
};



enum realloc_strategy {
    ReallocStrategy_None            = 0,
    ReallocStrategy_Xrealloc        = 1,
    ReallocStrategy_Realloc         = 2,
    ReallocStrategy_MRemap_MayMove  = 3, // Pointers inside the buffer may get invalidated. May need to use offset
                                         // inside the buffer which are relative position independent from the start
                                         // of the buffer.
    ReallocStrategy_MRemap_KeepAddr = 4, // Try to keep the address the same. Pointers to the memory region
                                         // will not get invalidate. May fail more easily if the OS could not fit
                                         // the memory region in the same virtual address space
};

enum dealloc_strategy {
    DeallocStrategy_Free = 0,
    DeallocStrategy_Munmap = 1,
};




// @IMPORTANT @NOTE: a MemRef with a 0 rel_offset value should be considered invalid and not pointing
//  to something usefull. Implementations of memory allocators making use of this
//  `MemRef` should skip 1 byte after the creation in order to reserve the zero-eth offset.
//   Usually a MemRef is implemented with a index (relative offset)
typedef U32 mem_ref_t;

void* mem_mmap(size_t size);
void  mem_unmap(void *addr, size_t size);

void*
mem_alloc( enum alloc_strategy alloc_strategy,
           size_t size,
           size_t alignment );

void*
mem_realloc__release ( enum realloc_strategy realloc_strategy,
                       void  *old_addr,
                       size_t old_size,
                       size_t new_size,
                       size_t alignment );

#if MEMORY_LAYER_DEBUG_CODE
void*
mem_realloc__debug ( enum realloc_strategy realloc_strategy,
                     void  *old_addr,
                     size_t old_size,
                     size_t new_size,
                     size_t alignment );
#endif



#if MEMORY_LAYER_DEBUG_CODE
#  define mem_realloc(realloc_strategy, addr, old_size, new_size, alignment) \
    mem_realloc__debug((realloc_strategy), (addr), (old_size), (new_size), (alignment))
#else
#  define mem_realloc(realloc_strategy, addr, old_size, new_size, alignment) \
    mem_realloc__release((realloc_strategy), (addr), (old_size), (new_size), (alignment))
#endif

#if 0
void *
mem_realloc ( enum realloc_strategy realloc_strategy,
              void  *addr,
              size_t old_size,
              size_t new_size,
              size_t alignment);
#endif

void
mem_dealloc (enum dealloc_strategy dealloc_strategy,
             void *addr,
             size_t  buffer_size);


/* @TAG: Generic Allocation, malloc, realloc, free */

void*
xmalloc (size_t size);

void*
xrealloc (void *ptr, size_t newsize );




typedef struct MArenaAtomicAllocationContext
{
    bool32 failed;
    U32    staging_size;
} MArenaAtomicAllocationContext;

#define MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE (16)

typedef struct marena {
    /* For internal usage only */
    MArenaAtomicAllocationContext alloc_context;
    enum alloc_strategy   alloc_strategy;
    enum realloc_strategy realloc_strategy;
    enum dealloc_strategy dealloc_strategy;
    /* ------------------- */

    U32                   data_size;
    U32                   data_max_size;
    
    U8* buffer;
} marena_t;


struct marena    marena_new_aux        ( enum alloc_strategy alloc_strategy,
                                         enum realloc_strategy realloc_strategy,
                                         enum dealloc_strategy dealloc_strategy,
                                         U32 size );
struct marena    marena_new            ( U32 size, bool may_grow );
void             marena_del            ( struct marena *arena );

void             marena_pop_upto       ( struct marena *arena, mem_ref_t ref );
void             marena_fetch          ( struct marena *arena, mem_ref_t ref, void *output, U32 sizeof_elem );
void             marena_clear          ( struct marena *arena );


/* Beging an atomic allocation context, you can start building up data incrementally
   directly on the arena. When calling `marena_commit` the data built up to that
   moment if there wasn't any error is going to be commited updating the `stack_pointer`
   and making the data actually ""visible"" to the user by returning a valid `mem_ref_t` to
   it.
   If you want to abort an atomic allocation context call `marena_dismiss`
   The `marena_add_xxxx` functionality allows you to construct data incrementally. They
   all return a bool saying if the request successed. You can choose to handle
   the failure right away by calling `marena_dismiss`, or just pretend
   nothing happened and keep pushing to it, once you will call `marena_commit`
   the function will return you a `mem_ref_t = 0` since one of the allocation failed.

   Between `marena_add_xxx` calls no alignment will be performed from the stack allocator,
   if you want alignment for performance reasons you must ask it explicitly.
*/
void             marena_begin              (struct marena *arena);

bool             marena_add                (struct marena *arena, void *data, U32 sizeof_data );
bool             marena_add_null_data      (struct marena *arena, U32 sizeof_data, bool initialize_to_zero );
bool             marena_add_pointer        (struct marena *arena, void *pointer);
bool             marena_add_byte           (struct marena *arena, byte_t b );
bool             marena_add_char           (struct marena *arena, char c );
bool             marena_add_i8             (struct marena *arena, I8 i8 );
bool             marena_add_u8             (struct marena *arena, U8 u8 );
bool             marena_add_i16            (struct marena *arena, I16 i16 );
bool             marena_add_u16            (struct marena *arena, U16 u16 );
bool             marena_add_i32            (struct marena *arena, I32 i32 );
bool             marena_add_u32            (struct marena *arena, U32 u32 );
bool             marena_add_i64            (struct marena *arena, I64 i64 );
bool             marena_add_u64            (struct marena *arena, U64 u64 );
bool             marena_add_size_t         (struct marena *arena, size_t s );
bool             marena_add_usize          (struct marena *arena, usize us );
bool             marena_add_cstr           (struct marena *arena, char* cstr );
bool             marena_add_pstr32         (struct marena *arena, PStr32 *pstr32 );
bool             marena_add_str32_nodata   (struct marena *arena, Str32 str32 );
bool             marena_add_str32_withdata (struct marena *arena, Str32 str32 );
bool             marena_ask_alignment      (struct marena *arena, U32 alignment);


void             marena_dismiss            (struct marena *arena);
mem_ref_t        marena_commit             (struct marena *arena);






mem_ref_t marena_push                (struct marena *arena, void *data, U32 sizeof_data );
mem_ref_t marena_push_null_data      (struct marena *arena, U32 sizeof_data, bool initialize_to_zero );
mem_ref_t marena_push_pointer        (struct marena *arena, void *pointer);
mem_ref_t marena_push_byte           (struct marena *arena, byte_t b );
mem_ref_t marena_push_char           (struct marena *arena, char c );
mem_ref_t marena_push_i8             (struct marena *arena, I8 i8 );
mem_ref_t marena_push_u8             (struct marena *arena, U8 u8 );
mem_ref_t marena_push_i16            (struct marena *arena, I16 i16 );
mem_ref_t marena_push_u16            (struct marena *arena, U16 u16 );
mem_ref_t marena_push_i32            (struct marena *arena, I32 i32 );
mem_ref_t marena_push_u32            (struct marena *arena, U32 u32 );
mem_ref_t marena_push_i64            (struct marena *arena, I64 i64 );
mem_ref_t marena_push_u64            (struct marena *arena, U64 u64 );
mem_ref_t marena_push_size_t         (struct marena *arena, size_t s );
mem_ref_t marena_push_usize          (struct marena *arena, usize us );
mem_ref_t marena_push_cstr           (struct marena *arena, char* cstr );
mem_ref_t marena_push_pstr32         (struct marena *arena, PStr32 *pstr32 );
mem_ref_t marena_push_str32_nodata   (struct marena *arena, Str32 str32 );
mem_ref_t marena_push_str32_withdata (struct marena *arena, Str32 str32 );
mem_ref_t marena_push_alignment      (struct marena *arena, U32 alignment);







static inline void *
marena_unpack_ref__unsafe(struct marena *arena, mem_ref_t ref)
{
    assert(arena->buffer);
    assert(arena->data_size);
    assert(ref && ref >= MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE && ref < arena->data_size);


    {
        /* @NOTE(dparo) [Mon Nov 26 22:05:28 CET 2018]
       
           It is not very polite to ask to access a raw pointer
           while in the middle of a `marena_begin` call.
           Make sure to commit or discard before accessing raw pointers.
           If you fill that this restriction is too severe remove this assert. 
        */
        assert(arena->alloc_context.staging_size == 0);
    }
    
    if (ref && ref >= MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE && ref < arena->data_size)
    {
        return (void*) ((U8*) arena->buffer + ref);
    }
    else
    {
        return 0;
    }
}

__END_DECLS

#endif  /* HGUARD_77f71620e38a4ba09d5a55b0bc707a92 */
