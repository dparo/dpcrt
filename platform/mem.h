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
#ifndef PLATFORM__MEM_LAYER_H
#define PLATFORM__MEM_LAYER_H

#include "utils.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "types.h"
#include "pal.h"


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
                                         // inside the buffer which are relative position indipendent from the start
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
typedef uint32 mem_ref_t;

void*
mem_alloc( enum alloc_strategy alloc_strategy,
           size_t size,
           size_t alignment );

void*
mem_realloc__release ( enum realloc_strategy realloc_strategy,
                       ptr_t  old_addr,
                       size_t old_size,
                       size_t new_size,
                       size_t alignment );

#if MEMORY_LAYER_DEBUG_CODE
void*
mem_realloc__debug ( enum realloc_strategy realloc_strategy,
                     ptr_t  old_addr,
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
              ptr_t  addr,
              size_t old_size,
              size_t new_size,
              size_t alignment);
#endif

void
mem_dealloc (enum dealloc_strategy dealloc_strategy,
             ptr_t  *addr,
             size_t  buffer_size);


/* @TAG: Generic Allocation, malloc, realloc, free */

void*
xmalloc (size_t size);

void*
xrealloc (void *ptr, size_t newsize );

typedef struct marena {
    U32                   data_size;
    U32                   data_max_size;
    enum alloc_strategy   alloc_strategy;
    enum realloc_strategy realloc_strategy;
    enum dealloc_strategy dealloc_strategy;

    ptr_t buffer;
} marena_t;


#define MARENA_GUARANTEED_ALIGNMENT (64)


struct marena  marena_new_aux          ( enum alloc_strategy alloc_strategy,
                                         enum realloc_strategy realloc_strategy,
                                         enum dealloc_strategy dealloc_strategy,
                                         U32 size );
struct marena    marena_new            ( U32 size, bool may_grow );
void             marena_del            ( struct marena *arena );
mem_ref_t        marena_push           ( struct marena *arena, void *data, U32 sizeof_data );
mem_ref_t        marena_push_null_data ( struct marena *arena, U32 sizeof_data, bool initialize_to_zero );
mem_ref_t        marena_push_byte      ( struct marena *arena, byte_t b );
mem_ref_t        marena_push_char      ( struct marena *arena, char c );
mem_ref_t        marena_push_pointer   ( struct marena *arena, void *pointer );
mem_ref_t        marena_push_cstring   ( struct marena *arena, char *string );
void             marena_pop_upto       ( struct marena *arena, mem_ref_t ref );
void             marena_fetch          ( struct marena *arena, mem_ref_t ref, void *output, U32 sizeof_elem );
void             marena_clear          ( struct marena *arena );

static inline void *
marena_begin(struct marena *arena)
{
    return arena->buffer + MARENA_GUARANTEED_ALIGNMENT;
}

static inline void *
marena_end(struct marena *arena)
{
    return arena->buffer + arena->data_size;
}

static inline void *
marena_unpack_ref__unsafe(struct marena *arena, mem_ref_t ref)
{
    assert(ref && ref >= MARENA_GUARANTEED_ALIGNMENT);
    assert(arena->buffer);
    assert(arena->data_size);
    assert(ref < arena->data_size);
    return (ptr_t) ((uint8 *) arena->buffer + ref);
}

__END_DECLS

#endif  /* PLATFORM__MEM_LAYER_H */
