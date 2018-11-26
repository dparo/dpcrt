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

#include "mem.h"
#include <stdlib.h>
#include <stdio.h>



void *
xmalloc ( size_t size )
{
    void *result = malloc(size);
    if ( !result )
    {
        perror("MEM: Memory allocation failed");
        pal_abort();
    }
    return result;
}

void *
xcalloc ( size_t size )
{
    void *result = calloc(1, size);
    if ( !result )
    {
        perror("MEM: Memory allocation failed");
        pal_abort();
    }
    return result;
}


void *
xrealloc ( void *ptr,
           size_t newsize )
{
    void *result = realloc(ptr, newsize);
    if (!result)
    {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "MEM: Failed to reallocate %zu bytes of memory -> ERRNO", newsize);
        perror(err_msg);
    }
    return result;
}



#if __PAL_LINUX__
# include <unistd.h>
#endif

static inline void *
mem_sbrk(size_t size)
{
#if __PAL_LINUX__
    return sbrk((intptr_t) size);
#else
    invalid_code_path("sbrk is not a valid allocation strategy for Windows platforms");
    return 0;
#endif
}

void*
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
        return mem_sbrk(size);
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
                       ptr_t  old_addr,
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
                     ptr_t  old_addr,
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
             ptr_t  *addr, // Double indirection pointer to allows us to set the pointer to null
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
marena_can_realloc(struct marena *arena)
{
    return (arena->realloc_strategy != ReallocStrategy_None);
}


static inline bool
marena_realloc(struct marena *arena,
               U32 newsize)
{
    bool success = false;
    assert(marena_can_realloc(arena));

    const size_t alignment = 128;

    void *buffer = mem_realloc (  arena->realloc_strategy,
                                  arena->buffer,
                                  arena->data_max_size,
                                  (size_t) newsize,
                                  alignment);
    if ( buffer )
    {
        arena->buffer = buffer;
        arena->data_max_size = newsize;
        success = true;
    }
    return success;
}

static inline bool
marena_grow(struct marena *arena)
{
    U32 newsize;
    if ((arena->data_max_size << U32_LIT(1)) < arena->data_max_size)
    {
        newsize = U32_MAX;
        assert_msg(arena->data_max_size != newsize, "Assert that the arena was able to grow, eg we didn't hit the maximum memory usage");

        if (newsize == arena->data_max_size)
        {
            /* Finished the available space that we can fit in a U32*/
            return false;
        }
    }
    else
    {
        newsize = (U32) ((F32) arena->data_max_size * 1.25f) + 8 * (U32) PAGE_SIZE;
    }    
    return marena_realloc(arena, newsize);
}



static bool
marena_accomodate_for_size(struct marena *arena, U32 size)
{
    bool success = false;
    assert(arena);
    bool can_realloc = marena_can_realloc(arena);

    const size_t needed_data_size = arena->data_size + size;

    if ( needed_data_size >= (size_t)(arena->data_max_size))
    {
        // needs to grow
        if ( !can_realloc )
        {
            return false;
        }
        else
        {
            if (marena_grow(arena))
            {
                success = true;
            }
            else
            {
                success = false;
            }
        }
    }
#if __DEBUG
    else if (MEMORY_ARENA_ALWAYS_FORCE_REALLOC_AT_EVERY_PUSH & can_realloc)
    {
        // If the arena didn't really need to grow, and the macro
        //    is defined we're going to force a new grow
        return marena_realloc(arena, arena->data_max_size);
    }
#endif
    else
    {
        success = true;
    }
    return success;
}



struct marena
marena_new_aux ( enum alloc_strategy alloc_strategy,
                 enum realloc_strategy realloc_strategy,
                 enum dealloc_strategy dealloc_strategy,
                 U32 size )
{
    assert(size);
    struct marena marena = {0};
    size = ALIGN(size, MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE);
    const U32 alignment = 128;
    size = ALIGN(size, alignment);
    const size_t alloc_size = size;
    
    void *buffer = mem_alloc( alloc_strategy, alloc_size, alignment);
    
    if (buffer)
    {
        marena.buffer = buffer;
        // We're going to reserve the first 64 bytes
        // so we can return `mem_ref_t` (indices to the buffer)
        // that do not start at zero
        marena.data_size = MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE;
        marena.data_max_size = size;
        marena.alloc_strategy = alloc_strategy;
        marena.realloc_strategy = realloc_strategy;
        marena.dealloc_strategy = dealloc_strategy;
    }
    return marena;
}


struct marena
marena_new( U32 size, bool buffer_may_change_addr )
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
        mem_dealloc(arena->dealloc_strategy, &(arena->buffer), arena->data_max_size);
    }
    memclr(arena, sizeof(*arena));
}


static inline void
assert_valid_marena(struct marena *arena)
{
    (void) arena;
    assert(arena && arena->buffer && (arena->data_size != 0) && arena->data_max_size);
}


void
marena_clear(struct marena *arena)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size == 0);
    memclr(&arena->alloc_context, sizeof(arena->alloc_context));
    arena->data_size = MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE;
}


void
marena_pop_upto(struct marena *arena, mem_ref_t ref)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size == 0);
    
    assert(ref >= MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE);

    assert(ref < arena->data_size);
    if (ref < arena->data_size)
    {
        arena->data_size = ref;
    }
    else
    {
        assert_msg(0, "You're using a reference that is possibly invalid, since it points to buffer that does not currently exist\n");
    }
}


void
marena_begin (struct marena *arena)
{
    assert_valid_marena(arena);
    assert_msg(arena->alloc_context.staging_size == 0, "Previous push begins must finish");
    arena->alloc_context.staging_size = (arena->data_size);
}


void
marena_dismiss (struct marena *arena)
{
    assert_valid_marena(arena);
    /* @NOTE :: Copied from pop_upto function */
    assert(arena && (arena->data_size != 0));
    assert(arena->alloc_context.staging_size >= arena->data_size);

    memclr(&arena->alloc_context, sizeof(arena->alloc_context));
}


mem_ref_t
marena_commit (struct marena *arena)
{
    mem_ref_t ref = 0;

    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size >= arena->data_size);
    
    if (!arena->alloc_context.failed)
    {
        ref = arena->data_size;
        arena->data_size = arena->alloc_context.staging_size;
    }

    marena_dismiss(arena);

    return ref;
}


static bool
marena_add_failure(struct marena *arena)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);
    arena->alloc_context.failed = true;
    return false;
}

static inline bool
marena_would_overflow_stack_pointer(struct marena *arena,
                                    U32 sizeof_data)
{
    if ( sizeof_data > U32_MAX - arena->alloc_context.staging_size)
    {
        return true;
    }
    else
    {
        return false;
    }
}


static inline bool
marena_ensure_add_operation_is_possible(struct marena *arena,
                                        U32 sizeof_data_to_be_added)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if ((arena->alloc_context.failed == false)
        && (!marena_would_overflow_stack_pointer(arena, sizeof_data_to_be_added)))
    {
        if (marena_accomodate_for_size(arena, sizeof_data_to_be_added))
        {
            return true;
        }
    }
    return false;
}

bool
marena_add(struct marena *arena, void *data, U32 sizeof_data )
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, sizeof_data))
    {
        return marena_add_failure(arena);
    }
    
    memcpy((ptr_t) arena->buffer + arena->alloc_context.staging_size,
           data, sizeof_data);
    arena->alloc_context.staging_size += sizeof_data;
    
    return true;
}

bool
marena_add_null_data(struct marena *arena, U32 sizeof_data, bool initialize_to_zero )
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, sizeof_data))
    {
        return marena_add_failure(arena);
    }


    if (initialize_to_zero)
    {
        memclr((ptr_t) arena->buffer + arena->alloc_context.staging_size, sizeof_data);
    }
    arena->alloc_context.staging_size += sizeof_data;
    
    return true;
}

bool
marena_add_pointer(struct marena *arena, void *pointer)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);
    
    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(void*)))
    {
        return marena_add_failure(arena);
    }

    *((void **)arena->buffer + arena->alloc_context.staging_size) = pointer;
    
    arena->alloc_context.staging_size += (U32) sizeof(void*);
    return true;
}

bool
marena_add_byte(struct marena *arena, byte_t b)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);
    
    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(byte_t)))
    {
        return marena_add_failure(arena);
    }

    *((byte_t*) arena->buffer + arena->alloc_context.staging_size) = b;
    
    arena->alloc_context.staging_size += (U32) sizeof(byte_t);

    return true;
}

bool
marena_add_char (struct marena *arena, char c)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(char)))
    {
        return marena_add_failure(arena);
    }

    *((char*) arena->buffer + arena->alloc_context.staging_size) = c;
    arena->alloc_context.staging_size += (U32) sizeof(char);
    
    return true;
}



bool
marena_add_i8(struct marena *arena, I8 i8)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(I8)))
    {
        return marena_add_failure(arena);
    }

    *((I8*) arena->buffer + arena->alloc_context.staging_size) = i8;
    arena->alloc_context.staging_size += (U32) sizeof(I8);
    
    return true;
}


bool
marena_add_u8(struct marena *arena, U8 u8)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(U8)))
    {
        return marena_add_failure(arena);
    }

    *((U8*) arena->buffer + arena->alloc_context.staging_size) = u8;
    arena->alloc_context.staging_size += (U32) sizeof(U8);
    
    return true;
}




bool
marena_add_i16(struct marena *arena, I16 i16)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(I16)))
    {
        return marena_add_failure(arena);
    }

    *((I16*) arena->buffer + arena->alloc_context.staging_size) = i16;
    arena->alloc_context.staging_size += (U32) sizeof(I16);
    
    return true;
}


bool
marena_add_u16(struct marena *arena, U16 u16)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(U16)))
    {
        return marena_add_failure(arena);
    }

    *((U16*) arena->buffer + arena->alloc_context.staging_size) = u16;
    arena->alloc_context.staging_size += (U32) sizeof(U16);
    
    return true;
}





bool
marena_add_i32(struct marena *arena, I32 i32)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(I32)))
    {
        return marena_add_failure(arena);
    }

    *((I32*) arena->buffer + arena->alloc_context.staging_size) = i32;
    arena->alloc_context.staging_size += (U32) sizeof(I32);
    
    return true;
}


bool
marena_add_u32(struct marena *arena, U32 u32)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(U32)))
    {
        return marena_add_failure(arena);
    }

    *((U32*) arena->buffer + arena->alloc_context.staging_size) = u32;
    arena->alloc_context.staging_size += (U32) sizeof(U32);
    
    return true;
}




bool
marena_add_i64(struct marena *arena, I64 i64)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(I64)))
    {
        return marena_add_failure(arena);
    }

    *((I64*) arena->buffer + arena->alloc_context.staging_size) = i64;
    arena->alloc_context.staging_size += (U32) sizeof(I64);
    
    return true;
}


bool
marena_add_u64(struct marena *arena, U64 u64)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(U64)))
    {
        return marena_add_failure(arena);
    }

    *((U64*) arena->buffer + arena->alloc_context.staging_size) = u64;
    arena->alloc_context.staging_size += (U32) sizeof(U64);
    
    return true;
}


bool
marena_add_size_t(struct marena *arena, size_t s)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(size_t)))
    {
        return marena_add_failure(arena);
    }

    *((size_t*) arena->buffer + arena->alloc_context.staging_size) = s;
    arena->alloc_context.staging_size += (U32) sizeof(size_t);
    
    return true;
}

bool
marena_add_usize(struct marena *arena, usize us)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, (U32) sizeof(usize)))
    {
        return marena_add_failure(arena);
    }

    *((usize*) arena->buffer + arena->alloc_context.staging_size) = us;
    arena->alloc_context.staging_size += (U32) sizeof(us);
    
    return true;
}

bool
marena_add_cstr(struct marena *arena, char *cstr)
{
    bool result = true;
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    for (char *s = cstr; *s; s++)
    {
        result = marena_add_char(arena, *s);
        if (!result)
        {
            break;
        }
    }
    // Add the null-terminator
    result = marena_add_char(arena, '\0');
    return result;
}




bool
marena_add_pstr32( struct marena *arena, PStr32 *pstr32 )
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);
    
    return marena_add(arena, pstr32, (U32) sizeof(Str32Hdr) + (U32) pstr32->bufsize);
}


bool
marena_add_str32_nodata( struct marena *arena, Str32 str32 )
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);
 
    return marena_add(arena, &str32, (U32) sizeof(Str32Hdr));
}


bool
marena_add_str32_withdata( struct marena *arena, Str32 str32 )
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);
    assert(str32.bufsize > 0);

    bool result = marena_add(arena, &str32, (U32) sizeof(Str32Hdr));
    if (result)
    {
        result = marena_add(arena, (ptr_t) &str32 + sizeof(Str32Hdr), (U32) str32.bufsize);
    }
    return result;
}



bool
marena_ask_alignment(struct marena *arena, U32 alignment)
{
    assert(arena && (arena->data_size != 0));
    const usize curr_addr = (usize) arena->buffer + arena->data_size;
    const usize aligned_addr = (usize) ALIGN(curr_addr, (usize) alignment);
    const U32 required_bytes_for_alignment = (U32) (aligned_addr - curr_addr);
    return marena_add_null_data(arena, required_bytes_for_alignment, true);
}


#define MARENA_PUSH_WRAPPER_DEF(...)    \
        marena_begin(arena);                      \
        __VA_ARGS__;                              \
        return marena_commit(arena);              \


mem_ref_t marena_push                (struct marena *arena, void *data, U32 sizeof_data )               { MARENA_PUSH_WRAPPER_DEF(marena_add                (arena, data, sizeof_data)) }
mem_ref_t marena_push_null_data      (struct marena *arena, U32 sizeof_data, bool initialize_to_zero )  { MARENA_PUSH_WRAPPER_DEF(marena_add_null_data      (arena, sizeof_data, initialize_to_zero)) }
mem_ref_t marena_push_pointer        (struct marena *arena, void *pointer)                              { MARENA_PUSH_WRAPPER_DEF(marena_add_pointer        (arena, pointer)) }
mem_ref_t marena_push_byte           (struct marena *arena, byte_t b )                                  { MARENA_PUSH_WRAPPER_DEF(marena_add_byte           (arena, b )) }
mem_ref_t marena_push_char           (struct marena *arena, char c )                                    { MARENA_PUSH_WRAPPER_DEF(marena_add_char           (arena, c )) }
mem_ref_t marena_push_i8             (struct marena *arena, I8 i8 )                                     { MARENA_PUSH_WRAPPER_DEF(marena_add_i8             (arena, i8 )) }
mem_ref_t marena_push_u8             (struct marena *arena, U8 u8 )                                     { MARENA_PUSH_WRAPPER_DEF(marena_add_u8             (arena, u8 )) }
mem_ref_t marena_push_i16            (struct marena *arena, I16 i16 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_i16            (arena, i16 )) }
mem_ref_t marena_push_u16            (struct marena *arena, U16 u16 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_u16            (arena, u16 )) }
mem_ref_t marena_push_i32            (struct marena *arena, I32 i32 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_i32            (arena, i32 )) }
mem_ref_t marena_push_u32            (struct marena *arena, U32 u32 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_u32            (arena, u32 )) }
mem_ref_t marena_push_i64            (struct marena *arena, I64 i64 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_i64            (arena, i64 )) }
mem_ref_t marena_push_u64            (struct marena *arena, U64 u64 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_u64            (arena, u64 )) }
mem_ref_t marena_push_size_t         (struct marena *arena, size_t s )                                  { MARENA_PUSH_WRAPPER_DEF(marena_add_size_t         (arena, s )) }
mem_ref_t marena_push_usize          (struct marena *arena, usize us )                                  { MARENA_PUSH_WRAPPER_DEF(marena_add_usize          (arena, us )) }
mem_ref_t marena_push_cstr           (struct marena *arena, char* cstr )                                { MARENA_PUSH_WRAPPER_DEF(marena_add_cstr           (arena, cstr )) }
mem_ref_t marena_push_pstr32         (struct marena *arena, PStr32 *pstr32 )                            { MARENA_PUSH_WRAPPER_DEF(marena_add_pstr32        (arena, pstr32 )) }
mem_ref_t marena_push_str32_nodata   (struct marena *arena, Str32 str32 )                               { MARENA_PUSH_WRAPPER_DEF(marena_add_str32_nodata   (arena, str32 )) }
mem_ref_t marena_push_str32_withdata (struct marena *arena, Str32 str32 )                               { MARENA_PUSH_WRAPPER_DEF(marena_add_str32_withdata (arena, str32 )) }
mem_ref_t marena_push_alignment      (struct marena *arena, U32 alignment)                              { MARENA_PUSH_WRAPPER_DEF(marena_ask_alignment      (arena, alignment)) }
