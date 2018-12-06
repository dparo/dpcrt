#include "dpcrt_allocators.h"
#include "dpcrt_mem.h"


#ifdef FREELIST_DISABLE_DEBUG_ASSERTION
#  define internal_assert(...)
#else
#  define internal_assert(...) assert(__VA_ARGS__)
#endif


/* ##########################################################################
   FreeList Implementation
   ########################################################################## */

#define FREELIST_RESERVED_CHUNK_HEADER_SIZE ( ALIGN(sizeof(FreeListChunk), 128) )


static FreeListChunk *
freelist_get_chunk_from_addr(FreeList *freelist,
                             void *_addr)
{
    FreeListChunk *result = NULL;

    FreeListChunk *it = freelist->first_chunk;

    U8 *addr = _addr;

    if (0 != ((usize)(addr - FREELIST_RESERVED_CHUNK_HEADER_SIZE)
              % freelist->block_size))
    {
        return NULL;
    }

    do {

        if ((addr > ((U8*) it)) && (addr < (((U8*) it) + freelist->chunk_size)))
        {
            result = it;
            break;
        }

    } while ((it = it->next_chunk));

    return result;
}

static inline void
assert_block_fits_in_chunk(FreeList *freelist,
                           FreeListChunk *chunk,
                           FreeListBlock *block)
{
    (void) freelist, (void) chunk, (void) block;
    if (block)
    {
        internal_assert(((U8*) block + freelist->block_size)
                        > ((U8*) chunk + freelist->chunk_size));
    }
}

static inline FreeListBlock *
get_next_adjacent_block(FreeList *freelist,
                        FreeListChunk *chunk,
                        FreeListBlock *block)
{
    FreeListBlock *result = (FreeListBlock*)
        ((U8*) block + freelist->block_size);

    if (((U8*) result + freelist->block_size) > ((U8*) chunk + freelist->chunk_size))
    {
        result = NULL;
    }

    return result;
}


static inline void
mark_block_as_used(FreeList *freelist,
                   FreeListChunk *chunk,
                   FreeListBlock *block)
{
    internal_assert(chunk->next_block == block);

    FreeListBlock *next_block = NULL;

    if (block->following_blocks_are_all_free)
    {
        next_block = get_next_adjacent_block(freelist, chunk, block);
        if (next_block)
        {
            next_block->following_blocks_are_all_free = true;
        }
    }
    else
    {
        next_block = block->next_block;
    }

    assert_block_fits_in_chunk(freelist, chunk, next_block);
    chunk->next_block = next_block;

    if (next_block)
    {
        memclr(next_block, freelist->block_size);
    }
}

static inline void
mark_block_as_avail(FreeList *freelist,
                    FreeListChunk *chunk,
                    FreeListBlock *block)
{
    internal_assert(chunk->next_block == block);

    {
        *block = (FreeListBlock) {0};
        block->following_blocks_are_all_free = false;
        block->next_block = chunk->next_block;
    }

    assert_block_fits_in_chunk(freelist, chunk, block);
    assert_block_fits_in_chunk(freelist, chunk, block->next_block);
    chunk->next_block = block;
}


static inline void
freelist_init_chunk(FreeListChunk *chunk)
{
    chunk->next_chunk    = NULL;
    chunk->next_block    = (FreeListBlock*) ((U8*) chunk
                                             + FREELIST_RESERVED_CHUNK_HEADER_SIZE);
    chunk->next_block->following_blocks_are_all_free = true;
}

static inline FreeListChunk *
freelist_new_chunk(U32 chunk_size)
{
    assert((chunk_size % PAGE_SIZE) == 0);
    FreeListChunk *newchunk = (FreeListChunk*) mem_mmap(chunk_size);

    if (newchunk)
    {
        freelist_init_chunk(newchunk);
    }

    return newchunk;
}



static inline FreeListChunk *
freelist_chain_new_chunk(FreeListChunk *prev_chunk,
                         U32 chunk_size)
{
    FreeListChunk *newchunk = freelist_new_chunk(chunk_size);

    if (prev_chunk)
    {
        prev_chunk->next_chunk = newchunk;
    }

    return newchunk;
}



void
freelist_free(FreeList *freelist, void *_addr)
{
    FreeListChunk *chunk = freelist_get_chunk_from_addr(freelist, _addr);
    assert(chunk);

    if (chunk)
    {
        FreeListBlock *block = (FreeListBlock *) _addr;
        mark_block_as_avail(freelist, chunk, block);
    }
}




void
freelist_clear(FreeList *freelist)
{
    assert(freelist->first_chunk);
    FreeListChunk *chunk = freelist->first_chunk;

    while(!(chunk->next_chunk))
    {
        FreeListChunk *next = chunk->next_chunk;
        freelist_init_chunk(chunk);
        chunk = next;
    }

    chunk->next_chunk = NULL;
}


void
freelist_del(FreeList *freelist)
{
    assert(freelist->first_chunk);
    FreeListChunk *chunk = freelist->first_chunk;

    while(!(chunk->next_chunk))
    {
        FreeListChunk *next = chunk->next_chunk;
        mem_unmap(chunk, freelist->chunk_size);
        chunk = next;
    }

    chunk->next_chunk = NULL;
}


void *
freelist_alloc(FreeList *freelist, U16 size)
{
    assert(freelist->first_chunk);
    void *result = NULL;
    if (size > freelist->block_size)
    {
        return NULL;
    }

    /* Find the chunk where we can allocate */
    FreeListChunk *chunk = freelist->first_chunk;

    bool full = false;

    while(!(chunk->next_chunk))
    {
        if (!chunk->next_chunk)
        {
            full = true;
            break;
        }
        chunk = chunk->next_chunk;
    }

    /* Check if we found a valid chunk, otherwise allocate a new one (if we can ...) */
    {
        if (full)
        {
            if (!freelist->allocate_more_chunks_on_demand)
            {
                return NULL;
            }
            else
            {
                FreeListChunk *newchunk = freelist_chain_new_chunk(chunk, freelist->chunk_size);

                if (!newchunk)
                {
                    return NULL;
                }

                chunk = newchunk;
            }

        }
    }


    /* Now we have a valid allocated chunk, let's use it for the allocation */
    {
        internal_assert(chunk);
        internal_assert(chunk->next_block);

        result = (void*) chunk->next_block;
        mark_block_as_used(freelist, chunk, chunk->next_block);
    }

    internal_assert(result);

    return result;
}

bool
freelist_init_aux(FreeList *freelist,
                  U32 chunk_size,
                  U16 block_size,
                  bool8 allocate_more_chunks_on_demand )
{
    bool result = true;

    assert_msg(chunk_size >= (2 * FREELIST_RESERVED_CHUNK_HEADER_SIZE), "Make sure to ask for a reasonable chunk size");
    assert_msg(block_size >= sizeof(void*), "Make sure to ask for a reasonable block_size");

    chunk_size = (U32) PAGE_ALIGN(chunk_size);
    block_size = (U16) ALIGN(block_size, sizeof(void*));

    *freelist                                = (FreeList) {0};
    freelist->chunk_size                     = chunk_size;
    freelist->block_size                     = block_size;
    freelist->allocate_more_chunks_on_demand = allocate_more_chunks_on_demand;


    freelist->first_chunk                    = freelist_new_chunk(chunk_size);

    if (!freelist->first_chunk)
    {
        result = false;
    }

    return result;
}

bool
freelist_init(FreeList *freelist, U16 block_size)
{
    const bool allocate_more_chunks_on_demand = true;

    return freelist_init_aux(freelist,
                             (U32) U16_MAX + 1,
                             block_size,
                             allocate_more_chunks_on_demand);
}
