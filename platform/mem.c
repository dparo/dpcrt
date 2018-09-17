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

#include "mem.h"
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"

void *
xmalloc ( size_t size )
{
    void *result = malloc(size);
    if ( !result )
        perror("MEM: Memory allocation failed");
    return result;
}

void *
xcalloc ( size_t size )
{
    void *result = calloc(1, size);
    if ( !result )
        perror("MEM: Memory allocation failed");
    return result;
}


void *
xrealloc ( void *ptr,
           size_t newsize )
{
    void *result = realloc(ptr, newsize);
    if (!result) {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "MEM: Failed to reallocate %zu bytes of memory -> ERRNO", newsize);
        perror(err_msg);
    }
    return result;
}


void *
mem_alloc( enum alloc_strategy alloc_strategy, size_t size, size_t alignment)
{
    switch (alloc_strategy)
    {

    default: { invalid_code_path(); } break;

    case AllocStrategy_Xmalloc: {
        return xmalloc(size);
    } break;

    case AllocStrategy_Malloc: {
        return malloc(size);
    } break;

    case AllocStrategy_Xcalloc: {
        return xcalloc(size);
    } break;

    case AllocStrategy_Calloc: {
        return calloc(1, size);
    } break;

    case AllocStrategy_AlignedAlloc: {
        return aligned_alloc(alignment, size);
    } break;

    case AllocStrategy_Mmap: {
        alignment = PAGE_ALIGN(alignment);
        void *addr = 0;
        const enum page_prot_flags prot = PAGE_PROT_READ | PAGE_PROT_WRITE;
        const enum page_type_flags type = PAGE_PRIVATE | PAGE_ANONYMOUS;
        size = PAGE_ALIGN(size);
        return pal_mmap_memory( addr, size, prot, type );
    } break;

    case AllocStrategy_Sbrk: {
        return sbrk(size);
    } break;
    }
    return NULL;
}





static inline void *
remap_keepAddr(void *old_addr, size_t old_size, size_t new_size)
{
    void *new_addr = 0; // No Fixed address, cleared to 0
    const enum page_remap_flags flags = PAGE_REMAP_NONE;
    new_size = PAGE_ALIGN(new_size);
    return pal_mremap( old_addr, old_size, new_addr, new_size, flags );
}

void *
mem_realloc__release ( enum realloc_strategy realloc_strategy,
                       void  *old_addr,
                       size_t old_size,
                       size_t new_size,
                       size_t alignment )
{
    UNUSED(alignment);

    switch (realloc_strategy)
    {

    default: { invalid_code_path(); } break;

    case ReallocStrategy_None: {
        if (new_size <= old_size) {
            // ReAllocation can fit inside the previosly already allocated block,
            //    simulate a successull reallocation
            return old_addr;
        } else {
            // We cannot reallocate, and the previous block is not big enough
            return NULL;
        }
    } break;

    case ReallocStrategy_Xrealloc: {
        return xrealloc(old_addr, new_size);
    } break;

    case ReallocStrategy_Realloc: {
        return realloc(old_addr, new_size);
    } break;


    case ReallocStrategy_MRemap_MayMove: {
        alignment = PAGE_ALIGN(alignment);
        void *new_addr = 0; // No Fixed address, cleared to 0
        const enum page_remap_flags flags = PAGE_REMAP_MAYMOVE;
        new_size = PAGE_ALIGN(new_size);
        return pal_mremap( old_addr, old_size, new_addr, new_size, flags );
    } break;

    case ReallocStrategy_MRemap_KeepAddr: {
        alignment = PAGE_ALIGN(alignment);
        return remap_keepAddr(old_addr, old_size, new_size);
    } break;

    }

    return NULL;
}


#if MEMORY_LAYER_DEBUG_CODE
// Memory realloc debug code. A new addr is always guaranteed
// to be generated except for the `realloc_none` & `mremap keepaddr` flag.
//  This is a convenient function to test if some pointers
//   after the reallocation are still pointing to the wrong
//   memory region.
void *
mem_realloc__debug ( enum realloc_strategy realloc_strategy,
                     void  *old_addr,
                     size_t old_size,
                     size_t new_size,
                     size_t alignment )
{

    switch (realloc_strategy)
    {

    default: { invalid_code_path(); } break;

    case ReallocStrategy_None: {
        if (new_size <= old_size) {
            // ReAllocation can fit inside the previosly already allocated block,
            //    simulate a successull reallocation
            return old_addr;
        } else {
            // We cannot reallocate, and the previous block is not big enough
            return NULL;
        }
    } break;

    case ReallocStrategy_Xrealloc: {
        void *new_addr = xmalloc(new_size);
        memcpy(new_addr, old_addr, old_size);
        free(old_addr);
        return new_addr;
    } break;

    case ReallocStrategy_Realloc: {
        void *new_addr = malloc(new_size);
        memcpy(new_addr, old_addr, old_size);
        free(old_addr);
        return new_addr;
    } break;


    case ReallocStrategy_MRemap_MayMove: {
        // In this case we will try to always force a move
        // to a new memory region
        alignment = PAGE_ALIGN(alignment);
        const enum alloc_strategy alloc_strategy = AllocStrategy_Mmap;
        void *new_addr = mem_alloc(alloc_strategy, new_size, alignment);
        memcpy(new_addr, old_addr, old_size);
#if MEMORY_LAYER_MREMAP_PROTECT_OLD_MMAPED_REGION
        const bool was_protected = pal_mprotect(old_addr, old_size, PAGE_PROT_NONE);
        assert(was_protected); // debug code we can simply assert here
#else
        mem_dealloc(DeallocStrategy_Munmap, & old_addr, old_size);
#endif
        
        return new_addr;
    } break;

    case ReallocStrategy_MRemap_KeepAddr: {
        alignment = PAGE_ALIGN(alignment);
        return remap_keepAddr(old_addr, old_size, new_size);
    } break;

    }

    return NULL;
}
#endif



void
mem_dealloc (enum dealloc_strategy dealloc_strategy,
             void  **addr, // Double indirection pointer to allows us to set the pointer to null
             size_t  size)
{
    switch (dealloc_strategy)
    {

    default: { invalid_code_path(); } break;

    case DeallocStrategy_Free: {
        free(*addr);
        *addr = NULL;
    } break;

    case DeallocStrategy_Munmap: {
        size = PAGE_ALIGN(size);
        int result = pal_munmap(*addr, size);
        (void) result;
        assert(result == 0);
        *addr = NULL;
    } break;

    }
}





static inline bool
marena__can_realloc(struct marena *arena)
{
    return (arena->realloc_strategy != ReallocStrategy_None);
}


static inline bool
marena__realloc(struct marena *arena,
                size_t size)
{
    bool success = false;
    assert(marena__can_realloc(arena));

    void *buffer = mem_realloc (  arena->realloc_strategy,
                                  arena->buffer,
                                  arena->data_max_size,
                                  size,
                                  MARENA_GUARANTEED_ALIGNMENT);
    if ( buffer )
    {
        arena->buffer = buffer;
        arena->data_max_size = size;
        success = true;
    }
    return success;
}

static inline bool
marena__grow(struct marena *arena)
{
    const size_t growsize = arena->data_max_size * 2;
    return marena__realloc(arena, growsize);
}

static inline void *
marena__push_unsafe(struct marena *arena, void *data, size_t sizeof_data)
{
    assert(arena->buffer && (arena->data_size != 0));
    void *dest = arena->buffer + arena->data_size;
    memcpy(dest, data, sizeof_data);
    arena->data_size += sizeof_data;
    return dest;
}

static inline mem_ref_t
marena__push_null_data__unsafe(struct marena *arena, size_t sizeof_data, bool initialize_to_zero)
{
    assert(arena->buffer && (arena->data_size != 0));
    mem_ref_t result = (arena->data_size);

    if (sizeof_data && initialize_to_zero)
    {
        memclr((uint8*) arena->buffer + arena->data_size, sizeof_data);
    }
    arena->data_size += sizeof_data;
    return result;
}    

static inline mem_ref_t
marena__push_byte__unsafe(struct marena *arena, uint8_t b)
{
    assert(arena->buffer && (arena->data_size != 0));
    mem_ref_t result = arena->data_size;
    arena->buffer[arena->data_size] = b;
    arena->data_size += sizeof(b);
    return result;
}

static inline mem_ref_t
marena__push_pointer__unsafe(struct marena *arena, void *pointer)
{
    assert(arena->buffer && (arena->data_size != 0));
    mem_ref_t result = arena->data_size;
    void **tmp = (void **)(&arena->buffer[arena->data_size]);
    *tmp = pointer;
    arena->data_size += sizeof(pointer);
    return result;
}


static inline mem_ref_t
marena__push_char__unsafe(struct marena *arena, char c)
{
    assert(arena->buffer && (arena->data_size != 0));
    return marena__push_byte__unsafe(arena, (uint8_t) c);
}



bool
marena__try_grow(struct marena *arena, size_t sizeof_data)
{
    bool success = false;
    assert(arena);
    bool can_realloc = marena__can_realloc(arena);

    const size_t needed_data_size = arena->data_size + sizeof_data;

    if ( needed_data_size >= (size_t)(arena->data_max_size))
    {
        // needs to grow
        if ( !can_realloc )
        {
            return false;
        }
        else
        {
            if (marena__grow(arena))
            {
                assert(arena->data_size + sizeof_data < arena->data_max_size);
                success = true;
            }
            else
            {
                success = false;
            }
        }
    }
#if __DEBUG
    else if (MEMORY_ARENA_ALWAYS_FORCE_REALLOC_AT_EVERY_PUSH & can_realloc) {
        // If the arena didn't really need to grow, and the macro
        //    is defined we're going to force a new grow
        return marena__realloc(arena, arena->data_max_size);
    }
#endif
    else {
        success = true;
    }
    return success;
}





struct marena
marena_new_aux ( enum alloc_strategy alloc_strategy,
                 enum realloc_strategy realloc_strategy,
                 enum dealloc_strategy dealloc_strategy,
                 size_t size )
{
    assert(size);
    struct marena result = {0};
    size = ALIGN(size, MARENA_GUARANTEED_ALIGNMENT);
    const size_t alignment  = MARENA_GUARANTEED_ALIGNMENT;
    const size_t alloc_size = size;

    void *buffer = mem_alloc( alloc_strategy, alloc_size, alignment);
    
    if (buffer)
    {
        result.buffer = buffer;
        // We're going to reserve the first 64 bytes
        // so we can return `mem_ref_t` (indices to the buffer)
        // that do not start at zero
        result.data_size = 64;
        result.data_max_size = size;
        result.alloc_strategy = alloc_strategy;
        result.realloc_strategy = realloc_strategy;
        result.dealloc_strategy = dealloc_strategy;

    }
    return result;
}


struct marena
marena_new( size_t size, bool buffer_may_change_addr )
{
    const enum alloc_strategy alloc_strategy = AllocStrategy_Mmap;
    const enum realloc_strategy realloc_strategy =  buffer_may_change_addr
        ? ReallocStrategy_MRemap_MayMove
        : ReallocStrategy_MRemap_KeepAddr;
    const enum dealloc_strategy dealloc_strategy = DeallocStrategy_Munmap;

    return marena_new_aux ( alloc_strategy, realloc_strategy, dealloc_strategy, size );
}


void
marena_del(struct marena *arena)
{
    assert(arena);
    if ( arena )
    {
        mem_dealloc(arena->dealloc_strategy, (void **) &(arena->buffer), arena->data_max_size);
    }
    memclr(arena, sizeof(*arena));
}


static inline size_t
marena_get_extra_bytes_for_alignment(struct marena *arena,
                                     size_t sizeof_data)
{
    assert(arena && (arena->data_size != 0));
    const uint8_t *curr_addr = (uint8_t*) arena->buffer + arena->data_size;
    const uint8_t *aligned_addr = (void*) ALIGN((size_t)curr_addr, sizeof_data);

    const size_t required_bytes_for_alignment = aligned_addr - curr_addr;
    return required_bytes_for_alignment;
}

mem_ref_t
marena_push(struct marena *arena, void *data, size_t sizeof_data)
{
    assert(arena && (arena->data_size != 0));
    assert(data);
    mem_ref_t result = 0;
    const size_t required_bytes_for_alignment = marena_get_extra_bytes_for_alignment(arena, sizeof_data);

    if (marena__try_grow(arena, sizeof_data + required_bytes_for_alignment)) {
# if MEMORY_LAYER_DEBUG_CODE
        const bool initialize_alignment_bytes = true;
# else
        const bool initialize_alignment_bytes = false;
# endif
        marena__push_null_data__unsafe(arena, required_bytes_for_alignment, initialize_alignment_bytes);
        void *abs_addr = marena__push_unsafe(arena, data, sizeof_data);
        result = (mem_ref_t) ((uint8_t*) abs_addr - (uint8_t*) arena->buffer);
    }

    return result;
}

mem_ref_t
marena_push_null_data (struct marena *arena, size_t sizeof_data, bool initialize_to_zero)
{
    assert(arena && (arena->data_size != 0));
    mem_ref_t result = 0;

    if (marena__try_grow(arena, sizeof_data))
    {
#if __DEBUG
        initialize_to_zero = true;
#endif
        result = marena__push_null_data__unsafe(arena, sizeof_data, initialize_to_zero);
    }

    return result;
}



mem_ref_t
marena_push_byte   (struct marena *arena, byte_t b)
{
    assert(arena && (arena->data_size != 0));

    mem_ref_t result = 0;

    if (marena__try_grow(arena, sizeof(b))) {
        result = marena__push_byte__unsafe(arena, b);
    }

    return result;
}

mem_ref_t
marena_push_char   (struct marena *arena, char c)
{
    assert(arena && (arena->data_size != 0));
    mem_ref_t result = 0;

    if (marena__try_grow(arena, sizeof(c))) {
        result = marena__push_char__unsafe(arena, c);
    }

    return result;

}

mem_ref_t
marena_push_pointer   (struct marena *arena, void *pointer)
{
    assert(arena && (arena->data_size != 0));
    mem_ref_t result = 0;

    if (marena__try_grow(arena, sizeof(pointer))) {
        result = marena__push_pointer__unsafe(arena, pointer);
    }

    return result;
}


mem_ref_t
marena_push_string (struct marena *arena, char *string)
{
    assert(arena && (arena->data_size != 0));
    assert(string);
    // During pushing the buffer data may reallocate
    //    we need to store relative addr first

    // @TODO @TODO Check this, Do we need it ??
    uint32_t initial_size = arena->data_size;

    mem_ref_t result = marena_push_char(arena, string[0]);
    if ( result ) {
        mem_ref_t temp = 0;
        for( char *c = string + 1; *c != 0; c ++ ) {
            temp = marena_push_char(arena, *c);
            if ( !temp ) { break; }
        }
        if ( *(string + 1) && temp == 0 ) {
            // If we ever fail to push even a single character we amend
            //    all the previously pushed chars and return an invalid reference
            marena_pop_upto(arena, result);
            result = 0;
        }
    }
    return result;
}

void
marena_pop_upto(struct marena *arena, mem_ref_t ref)
{
    assert(ref >= MARENA_GUARANTEED_ALIGNMENT); // zero-th index is reserved
    assert(arena && (arena->data_size != 0));

    assert(ref < arena->data_size);
    if (ref < arena->data_size)
    {
        arena->data_size = ref;
    }
    else
    {
        assert_msg(0, "You're using a reference that is possibly invalidate, since it points to buffer that does not currently exist\n");
    }
}

void
marena_fetch (struct marena *arena, mem_ref_t ref, void *output, size_t sizeof_elem)
{
    assert(arena && (arena->data_size != 0));
    assert(ref >= MARENA_GUARANTEED_ALIGNMENT); // zero-th index is reserved
    void *abs_addr = &(arena->buffer[ref]);
    memcpy(output, abs_addr, sizeof_elem);
}

void
marena_clear(struct marena *arena)
{
    assert(arena && (arena->data_size != 0));
    arena->data_size = MARENA_GUARANTEED_ALIGNMENT;
}
