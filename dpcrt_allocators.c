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

#include "dpcrt_allocators.h"


#ifdef MPOOL_DISABLE_DEBUG_ASSERTION
#  define internal_assert(...)
#else
#  define internal_assert(...) assert(__VA_ARGS__)
#endif


/* ##########################################################################
   MPool Implementation
   ########################################################################## */

#define MPOOL_RESERVED_CHUNK_HEADER_SIZE ( ALIGN(sizeof(MPoolChunk), 128) )


static MPoolChunk *
mpool_get_chunk_from_addr(MPool *mpool,
                          void *_addr)
{
    MPoolChunk *result = NULL;

    MPoolChunk *it = mpool->first_chunk;

    U8 *addr = _addr;

    if (0 != ((usize)(addr - MPOOL_RESERVED_CHUNK_HEADER_SIZE)
              % mpool->block_size))
    {
        return NULL;
    }

    do {

        if ((addr > ((U8*) it)) && (addr < (((U8*) it) + mpool->chunk_size)))
        {
            result = it;
            break;
        }

    } while ((it = it->next_chunk));

    return result;
}

static inline void
assert_block_fits_in_chunk(MPool *mpool,
                           MPoolChunk *chunk,
                           MPoolBlock *block)
{
    (void) mpool, (void) chunk, (void) block;
    if (block)
    {
        internal_assert(((U8*) block + mpool->block_size)
                        <= ((U8*) chunk + mpool->chunk_size));
    }
}

static inline MPoolBlock *
get_next_adjacent_block(MPool *mpool,
                        MPoolChunk *chunk,
                        MPoolBlock *block)
{
    MPoolBlock *result = (MPoolBlock*)
        ((U8*) block + mpool->block_size);

    if (((U8*) result + mpool->block_size) > ((U8*) chunk + mpool->chunk_size))
    {
        result = NULL;
    }

    return result;
}


static inline void
mark_block_as_used(MPool *mpool,
                   MPoolChunk *chunk,
                   MPoolBlock *block)
{
    internal_assert(chunk->next_block == block);

    MPoolBlock *next_block = NULL;

    if (block->following_blocks_are_all_free)
    {
        next_block = get_next_adjacent_block(mpool, chunk, block);
        if (next_block)
        {
            next_block->following_blocks_are_all_free = true;
        }
    }
    else
    {
        next_block = block->next_block;
    }

    assert_block_fits_in_chunk(mpool, chunk, next_block);
    chunk->next_block = next_block;

    if (next_block)
    {
        memclr(block, mpool->block_size);
    }
}

static inline void
mark_block_as_avail(MPool *mpool,
                    MPoolChunk *chunk,
                    MPoolBlock *block)
{
    {
        *block = (MPoolBlock) {0};
        block->following_blocks_are_all_free = false;
        block->next_block = chunk->next_block;
    }

    assert_block_fits_in_chunk(mpool, chunk, block);
    assert_block_fits_in_chunk(mpool, chunk, block->next_block);
    chunk->next_block = block;
}


static inline void
mpool_init_chunk(MPoolChunk *chunk)
{
    chunk->next_chunk    = NULL;
    chunk->next_block    = (MPoolBlock*) ((U8*) chunk
                                          + MPOOL_RESERVED_CHUNK_HEADER_SIZE);
    chunk->next_block->following_blocks_are_all_free = true;
}

static inline MPoolChunk *
mpool_new_chunk(U32 chunk_size)
{
    assert((chunk_size % PAGE_SIZE) == 0);
    MPoolChunk *newchunk = (MPoolChunk*) mem_mmap(chunk_size);

    if (newchunk)
    {
        mpool_init_chunk(newchunk);
    }

    return newchunk;
}



static inline MPoolChunk *
mpool_chain_new_chunk(MPoolChunk *prev_chunk,
                      U32 chunk_size)
{
    MPoolChunk *newchunk = mpool_new_chunk(chunk_size);

    if (prev_chunk)
    {
        prev_chunk->next_chunk = newchunk;
    }

    return newchunk;
}



void
mpool_free(MPool *mpool, void *_addr)
{
    MPoolChunk *chunk = mpool_get_chunk_from_addr(mpool, _addr);
    assert(chunk);

    if (chunk)
    {
        MPoolBlock *block = (MPoolBlock *) _addr;
        mark_block_as_avail(mpool, chunk, block);
    }
}




void
mpool_clear(MPool *mpool)
{
    assert(mpool->first_chunk);
    MPoolChunk *chunk = mpool->first_chunk;

    while(chunk)
    {
        MPoolChunk *tmp = chunk->next_chunk;
        mpool_init_chunk(chunk);
        chunk = tmp;
    }
}


void
mpool_del(MPool *mpool)
{
    assert(mpool->first_chunk);
    MPoolChunk *chunk = mpool->first_chunk;

    while(chunk)
    {
        MPoolChunk *tmp = chunk->next_chunk;
        mem_unmap(chunk, mpool->chunk_size);
        chunk = tmp;
    }

    memclr(mpool, sizeof(*mpool));
}


void *
mpool_alloc(MPool *mpool, U16 size)
{
    assert(mpool->first_chunk);
    if (size > mpool->block_size)
    {
        return NULL;
    }

    /* Find the chunk where we can allocate */
    MPoolChunk *chunk   = mpool->first_chunk;

    while( chunk->next_chunk
           && (chunk->next_block == NULL))
    {
        chunk = chunk->next_chunk;
    }

    /* Check if we have blocks available in this block, if not try to allocate
       a new chunk if possible */
    if (chunk->next_block == NULL)
    {
        if (mpool->allocate_more_chunks_on_demand)
        {
            chunk = mpool_chain_new_chunk(chunk, mpool->chunk_size);
        }
        else
        {
            /* We can't find a valid chunk cuz all of them are full and we cannot
               chain a new one */
            chunk = NULL;
        }
    }

    void *result = NULL;

    if (chunk)
    {
        internal_assert(chunk->next_block);

        result = (void*) chunk->next_block;
        mark_block_as_used(mpool, chunk, chunk->next_block);
        internal_assert(result);
    }

    return result;
}

bool
mpool_init_aux(MPool *mpool,
               U32 chunk_size,
               U16 block_size,
               bool8 allocate_more_chunks_on_demand )
{
    bool result = true;

    assert(block_size < chunk_size);
    assert_msg(chunk_size >= (2 * MPOOL_RESERVED_CHUNK_HEADER_SIZE), "Make sure to ask for a reasonable chunk size");
    assert_msg(block_size >= sizeof(void*), "Make sure to ask for a reasonable block_size");

    chunk_size = (U32) PAGE_ALIGN(chunk_size);
    block_size = (U16) ALIGN(block_size, sizeof(void*));

    *mpool                                = (MPool) {0};
    mpool->chunk_size                     = chunk_size;
    mpool->block_size                     = block_size;
    mpool->allocate_more_chunks_on_demand = allocate_more_chunks_on_demand;


    mpool->first_chunk                    = mpool_new_chunk(chunk_size);

    if (!mpool->first_chunk)
    {
        result = false;
    }

    return result;
}

bool
mpool_init(MPool *mpool, U16 block_size)
{
    const bool allocate_more_chunks_on_demand = true;

    return mpool_init_aux(mpool,
                          (U32) KILOBYTES(64),
                          block_size,
                          allocate_more_chunks_on_demand);
}



























static inline bool
marena_can_realloc(MArena *arena)
{
    return (arena->realloc_strategy != ReallocStrategy_None);
}


static inline bool
marena_realloc(MArena *arena,
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
marena_grow(MArena *arena)
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
marena_accomodate_for_size(MArena *arena, U32 size)
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



MArena
marena_new_aux ( enum AllocStrategy alloc_strategy,
                 enum ReallocStrategy realloc_strategy,
                 enum DeallocStrategy dealloc_strategy,
                 U32 size )
{
    assert(size);
    MArena marena = {0};
    size = ALIGN(size, MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE);
    const U32 alignment = 128;
    size = ALIGN(size, alignment);
    const size_t alloc_size = size;

    void *buffer = mem_alloc( alloc_strategy, alloc_size, alignment);

    if (buffer)
    {
        marena.buffer = buffer;
        // We're going to reserve the first 64 bytes
        // so we can return `MemRef` (indices to the buffer)
        // that do not start at zero
        marena.data_size = MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE;
        marena.data_max_size = size;
        marena.alloc_strategy = alloc_strategy;
        marena.realloc_strategy = realloc_strategy;
        marena.dealloc_strategy = dealloc_strategy;
    }
    return marena;
}


MArena
marena_new( U32 size, bool buffer_may_change_addr )
{
    const enum AllocStrategy alloc_strategy = AllocStrategy_Mmap;
    const enum ReallocStrategy realloc_strategy =  buffer_may_change_addr
        ? ReallocStrategy_MRemap_MayMove
        : ReallocStrategy_MRemap_KeepAddr;
    const enum DeallocStrategy dealloc_strategy = DeallocStrategy_Munmap;

    return marena_new_aux ( alloc_strategy, realloc_strategy, dealloc_strategy, size );
}


void
marena_del(MArena *arena)
{
    assert(arena);
    if ( arena )
    {
        mem_dealloc(arena->dealloc_strategy, arena->buffer, arena->data_max_size);
        arena->buffer = NULL;
    }
    memclr(arena, sizeof(*arena));
}


static inline void
assert_valid_marena(MArena *arena)
{
    (void) arena;
    assert(arena && arena->buffer && (arena->data_size != 0) && arena->data_max_size);
}


void
marena_clear(MArena *arena)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size == 0);
    memclr(&arena->alloc_context, sizeof(arena->alloc_context));
    arena->data_size = MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE;
}


void
marena_pop_upto(MArena *arena, MemRef ref)
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
marena_fetch( MArena *arena, MemRef ref, void *output, U32 sizeof_elem )
{
    void *ptr = marena_unpack_ref__unsafe(arena, ref);
    assert(ptr);
    if (ptr)
    {
        memcpy(output, ptr, sizeof_elem);
    }
    else
    {
        memclr(output, sizeof_elem);
    }
}


void
marena_begin (MArena *arena)
{
    assert_valid_marena(arena);
    assert_msg(arena->alloc_context.staging_size == 0, "Previous push begins must finish");
    arena->alloc_context.staging_size = (arena->data_size);
}


void
marena_dismiss (MArena *arena)
{
    assert_valid_marena(arena);
    /* @NOTE :: Copied from pop_upto function */
    assert(arena && (arena->data_size != 0));
    assert(arena->alloc_context.staging_size >= arena->data_size);

    memclr(&arena->alloc_context, sizeof(arena->alloc_context));
}


MemRef
marena_commit (MArena *arena)
{
    MemRef ref = 0;

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
marena_add_failure(MArena *arena)
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);
    arena->alloc_context.failed = true;
    return false;
}

static inline bool
marena_would_overflow_stack_pointer(MArena *arena,
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
marena_ensure_add_operation_is_possible(MArena *arena,
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
marena_add(MArena *arena, U32 size, bool initialize_to_zero )
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, size))
    {
        return marena_add_failure(arena);
    }


    if (initialize_to_zero)
    {
        memclr((U8*) arena->buffer + arena->alloc_context.staging_size, size);
    }
    arena->alloc_context.staging_size += size;

    return true;
}


bool
marena_add_data(MArena *arena, void *data, U32 sizeof_data )
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    if (!marena_ensure_add_operation_is_possible(arena, sizeof_data))
    {
        return marena_add_failure(arena);
    }

    if (data)
    {
        memcpy((U8*) arena->buffer + arena->alloc_context.staging_size,
               data, sizeof_data);
    }

    arena->alloc_context.staging_size += sizeof_data;

    return true;
}

bool
marena_add_pointer(MArena *arena, void *pointer)
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
marena_add_byte(MArena *arena, byte_t b)
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
marena_add_char (MArena *arena, char c)
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
marena_add_i8(MArena *arena, I8 i8)
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
marena_add_u8(MArena *arena, U8 u8)
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
marena_add_i16(MArena *arena, I16 i16)
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
marena_add_u16(MArena *arena, U16 u16)
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
marena_add_i32(MArena *arena, I32 i32)
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
marena_add_u32(MArena *arena, U32 u32)
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
marena_add_i64(MArena *arena, I64 i64)
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
marena_add_u64(MArena *arena, U64 u64)
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
marena_add_size_t(MArena *arena, size_t s)
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
marena_add_usize(MArena *arena, usize us)
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
marena_add_cstr(MArena *arena, char *cstr)
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
    if (result)
    {
        // Add the null-terminator
        result = marena_add_char(arena, '\0');
    }
    return result;
}




bool
marena_add_pstr32( MArena *arena, PStr32 *pstr32 )
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    return marena_add_data(arena, pstr32, (U32) sizeof(Str32Hdr) + (U32) pstr32->bufsize);
}


bool
marena_add_str32_nodata( MArena *arena, Str32 str32 )
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);

    return marena_add_data(arena, &str32, (U32) sizeof(Str32Hdr));
}


bool
marena_add_str32_withdata( MArena *arena, Str32 str32 )
{
    assert_valid_marena(arena);
    assert(arena->alloc_context.staging_size != 0);
    assert(str32.bufsize > 0);

    bool result = marena_add_data(arena, &str32, (U32) sizeof(Str32Hdr));
    if (result)
    {
        result = marena_add_data(arena, (U8*) &str32 + sizeof(Str32Hdr), (U32) str32.bufsize);
    }
    return result;
}



bool
marena_ask_alignment(MArena *arena, U32 alignment)
{
    assert(arena && (arena->data_size != 0));
    const usize curr_addr = (usize) arena->buffer + arena->data_size;
    const usize aligned_addr = (usize) ALIGN(curr_addr, (usize) alignment);
    const U32 required_bytes_for_alignment = (U32) (aligned_addr - curr_addr);
    return marena_add(arena, required_bytes_for_alignment, true);
}


#define MARENA_PUSH_WRAPPER_DEF(...)            \
    marena_begin(arena);                        \
    __VA_ARGS__;                                \
    return marena_commit(arena);                \


MemRef marena_push                (MArena *arena, U32 size, bool initialize_to_zero )         { MARENA_PUSH_WRAPPER_DEF(marena_add                (arena, size, initialize_to_zero)) }
MemRef marena_push_data           (MArena *arena, void *data, U32 sizeof_data )               { MARENA_PUSH_WRAPPER_DEF(marena_add_data           (arena, data, sizeof_data)) }
MemRef marena_push_pointer        (MArena *arena, void *pointer)                              { MARENA_PUSH_WRAPPER_DEF(marena_add_pointer        (arena, pointer)) }
MemRef marena_push_byte           (MArena *arena, byte_t b )                                  { MARENA_PUSH_WRAPPER_DEF(marena_add_byte           (arena, b )) }
MemRef marena_push_char           (MArena *arena, char c )                                    { MARENA_PUSH_WRAPPER_DEF(marena_add_char           (arena, c )) }
MemRef marena_push_i8             (MArena *arena, I8 i8 )                                     { MARENA_PUSH_WRAPPER_DEF(marena_add_i8             (arena, i8 )) }
MemRef marena_push_u8             (MArena *arena, U8 u8 )                                     { MARENA_PUSH_WRAPPER_DEF(marena_add_u8             (arena, u8 )) }
MemRef marena_push_i16            (MArena *arena, I16 i16 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_i16            (arena, i16 )) }
MemRef marena_push_u16            (MArena *arena, U16 u16 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_u16            (arena, u16 )) }
MemRef marena_push_i32            (MArena *arena, I32 i32 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_i32            (arena, i32 )) }
MemRef marena_push_u32            (MArena *arena, U32 u32 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_u32            (arena, u32 )) }
MemRef marena_push_i64            (MArena *arena, I64 i64 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_i64            (arena, i64 )) }
MemRef marena_push_u64            (MArena *arena, U64 u64 )                                   { MARENA_PUSH_WRAPPER_DEF(marena_add_u64            (arena, u64 )) }
MemRef marena_push_size_t         (MArena *arena, size_t s )                                  { MARENA_PUSH_WRAPPER_DEF(marena_add_size_t         (arena, s )) }
MemRef marena_push_usize          (MArena *arena, usize us )                                  { MARENA_PUSH_WRAPPER_DEF(marena_add_usize          (arena, us )) }
MemRef marena_push_cstr           (MArena *arena, char* cstr )                                { MARENA_PUSH_WRAPPER_DEF(marena_add_cstr           (arena, cstr )) }
MemRef marena_push_pstr32         (MArena *arena, PStr32 *pstr32 )                            { MARENA_PUSH_WRAPPER_DEF(marena_add_pstr32        (arena, pstr32 )) }
MemRef marena_push_str32_nodata   (MArena *arena, Str32 str32 )                               { MARENA_PUSH_WRAPPER_DEF(marena_add_str32_nodata   (arena, str32 )) }
MemRef marena_push_str32_withdata (MArena *arena, Str32 str32 )                               { MARENA_PUSH_WRAPPER_DEF(marena_add_str32_withdata (arena, str32 )) }
MemRef marena_push_alignment      (MArena *arena, U32 alignment)                              { MARENA_PUSH_WRAPPER_DEF(marena_ask_alignment      (arena, alignment)) }
