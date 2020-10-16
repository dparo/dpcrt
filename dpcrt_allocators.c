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

#include <assert.h>
#include <memory.h>


#ifdef MPOOL_DISABLE_DEBUG_ASSERTION
#  define internal_assert(...)
#  define internal_assert_msg(...)
#else
#  define internal_assert(...) assert(__VA_ARGS__)
#  define internal_assert_msg(...) assert_msg(__VA_ARGS__)
#endif


#include "dpcrt_valgrind_memcheck.h"


typedef struct alloc_entry {
    size_t size;
    void *ptr;
} alloc_entry_t;

typedef struct malloc_miface {
    size_t num_allocs;
    alloc_entry_t *table;
} malloc_miface_t;

#define MALLOC_MIFACE_GET_HANDLE(self) ((malloc_miface_t *)((self)->instance_handle))

void *malloc_miface_new(struct miface *self, size_t size)
{
    malloc_miface_t *iface = MALLOC_MIFACE_GET_HANDLE(self);

    void *ptr = malloc(size);
    if (ptr) {
        void *newTable = realloc(iface->table, sizeof(alloc_entry_t) * (iface->num_allocs + 1));
        if (newTable) {
            iface->table = newTable;
            iface->table[iface->num_allocs].ptr = ptr;
            iface->table[iface->num_allocs].size = size;
            ++iface->num_allocs;
        } else {
            free(ptr);
        }
    }

    memset(ptr, 0, size);
    return ptr;
}

alloc_entry_t *_malloc_iface_query_alloc_entry(malloc_miface_t *iface, void *ptr)
{
    alloc_entry_t *entry = NULL;
    for (size_t i = 0; i < iface->num_allocs; i++) {
        if (iface->table[i].ptr == ptr) {
            entry = &iface->table[i];
            break;
        }
    }
    return entry;
}

void *malloc_miface_renew(struct miface *self, void *oldptr, size_t newsize)
{
    malloc_miface_t *iface = MALLOC_MIFACE_GET_HANDLE(self);

    if (oldptr == NULL)
        return malloc_miface_new(self, newsize);

    alloc_entry_t *entry = _malloc_iface_query_alloc_entry(iface, oldptr);
    assert(entry);

    void *newResourcePtr = realloc(oldptr, newsize);
    if (newResourcePtr) {
        entry->ptr = newResourcePtr;
        entry->size = newsize;
    }

    return newResourcePtr;
}

void malloc_miface_del(struct miface *self, void *oldptr)
{
    malloc_miface_t *iface = MALLOC_MIFACE_GET_HANDLE(self);

    alloc_entry_t *entry = NULL;
    for (size_t i = 0; i < iface->num_allocs; i++) {
        if (iface->table[i].ptr == oldptr) {
            entry = &iface->table[i];
            break;
        }
    }

    assert(entry);

#if _DEBUG
    memset(entry->ptr, 0xCC, entry->size);
#endif

    free(entry->ptr);
    entry->ptr = NULL;
    entry->size = 0;

    // Swap with the last allocation, we could have equivalently
    // swapped it with a random allocation excluding the current one.
    *entry = iface->table[iface->num_allocs - 1];

    void *newTable = realloc(iface->table, (iface->num_allocs - 1) * sizeof(alloc_entry_t));
    iface->table = newTable;
    --iface->num_allocs;
}

void malloc_miface_clear(struct miface *self)
{
    malloc_miface_t *iface = MALLOC_MIFACE_GET_HANDLE(self);
    for (size_t i = 0; i < iface->num_allocs; i++) {
        alloc_entry_t *entry = &iface->table[i];

#if _DEBUG
        memset(entry->ptr, 0xCC, entry->size);
#endif
        free(entry->ptr);
        entry->ptr = 0;
        entry->size = 0;
    }

    iface->num_allocs = 0;
    free(iface->table);
    iface->table = NULL;
}

miface_t make_malloc_based_allocator(void)
{
    miface_t result = {};
    malloc_miface_t *iface = malloc(sizeof(malloc_miface_t));
    if (iface) {
        iface->num_allocs = 0;
        iface->table = NULL;
    }

    result.instance_handle = (void *)iface;
    result.new = malloc_miface_new;
    result.renew = malloc_miface_renew;
    result.del = malloc_miface_del;
    result.clear = malloc_miface_clear;
    return result;
}







/* ##########################################################################
   MPool Implementation
   ########################################################################## */

#define MPOOL_RESERVED_CHUNK_HEADER_SIZE ( ALIGN(size_t, sizeof(MPoolChunk), 128) )


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
mpool__assert_block_fits_in_chunk(MPool *mpool,
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
mpool__get_next_adjacent_block(MPool *mpool,
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
mpool__mark_block_as_used(MPool *mpool,
                          MPoolChunk *chunk,
                          MPoolBlock *block)
{
    internal_assert(chunk->next_block == block);

    MPoolBlock *next_block = NULL;

    if (block->following_blocks_are_all_free)
    {
        next_block = mpool__get_next_adjacent_block(mpool, chunk, block);
        if (next_block)
        {
            next_block->following_blocks_are_all_free = true;
        }
    }
    else
    {
        next_block = block->next_block;
    }

    mpool__assert_block_fits_in_chunk(mpool, chunk, next_block);
    chunk->next_block = next_block;

    memclr(block, mpool->block_size);
    mpool->total_user_memory_usage += mpool->block_size;
}

static inline void
mpool__mark_block_as_avail(MPool *mpool,
                           MPoolChunk *chunk,
                           MPoolBlock *block)
{
    {
        *block = (MPoolBlock) {0};
        block->following_blocks_are_all_free = false;
        block->next_block = chunk->next_block;
    }

    mpool__assert_block_fits_in_chunk(mpool, chunk, block);
    mpool__assert_block_fits_in_chunk(mpool, chunk, block->next_block);
    chunk->next_block = block;

    mpool->total_user_memory_usage -= mpool->block_size;
}


static inline void
mpool__init_chunk(MPoolChunk *chunk)
{
    chunk->next_chunk    = NULL;
    chunk->next_block    = (MPoolBlock*) ((U8*) chunk
                                          + MPOOL_RESERVED_CHUNK_HEADER_SIZE);
    chunk->next_block->following_blocks_are_all_free = true;
}


static inline MPoolChunk *
mpool__new_chunk(U32 chunk_size)
{
    assert(IS_PAGE_ALIGNED(chunk_size));
    MPoolChunk *newchunk = (MPoolChunk*) mem_mmap(chunk_size);

    if (newchunk)
    {
        mpool__init_chunk(newchunk);
    }

    return newchunk;
}



static inline MPoolChunk *
mpool__chain_new_chunk(MPool *mpool,
                       MPoolChunk *prev_chunk)
{
    U32 chunk_size = mpool->chunk_size;

    MPoolChunk *newchunk = mpool__new_chunk(chunk_size);
    if (newchunk)
    {
        mpool->total_allocator_memory_usage += chunk_size;
    }

    if (prev_chunk)
    {
        prev_chunk->next_chunk = newchunk;
    }

    return newchunk;
}

static inline void
mpool__del_chunk(MPoolChunk *chunk_to_be_deleted,
                 U32 chunk_size)
{
    mem_unmap(chunk_to_be_deleted, chunk_size);
}


static inline void
mpool__del_chained_chunk(MPool *mpool,
                         MPoolChunk *prev_chunk,
                         MPoolChunk *chunk_to_be_deleted)
{
    prev_chunk->next_chunk = chunk_to_be_deleted->next_chunk;
    mpool->total_allocator_memory_usage -= mpool->chunk_size;
    mpool__del_chunk(chunk_to_be_deleted, mpool->chunk_size);
}



void
mpool_free(MPool *mpool, void *_addr)
{
    MPoolChunk *chunk = mpool_get_chunk_from_addr(mpool, _addr);
    assert(chunk);

    if (chunk)
    {
        MPoolBlock *block = (MPoolBlock *) _addr;
        mpool__mark_block_as_avail(mpool, chunk, block);
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
        mpool__init_chunk(chunk);
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
            chunk = mpool__chain_new_chunk(mpool, chunk);
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
        mpool__mark_block_as_used(mpool, chunk, chunk->next_block);
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
    block_size = ALIGN(U16, block_size, sizeof(void*));

    *mpool                                = (MPool) {0};
    mpool->chunk_size                     = chunk_size;
    mpool->block_size                     = block_size;
    mpool->allocate_more_chunks_on_demand = allocate_more_chunks_on_demand;


    mpool->first_chunk                    = mpool__new_chunk(chunk_size);

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




/* #############################################################################
   MFList Implementation
   #############################################################################
 */


#define MFLIST_RESERVED_CHUNK_HEADER_SIZE ( ALIGN(sizeof(MFListChunk), 128) )

enum MFListAllocCateg
{
    MFListAllocCateg_512  = 0,
    MFListAllocCateg_4K   = 1,
    MFListAllocCateg_32K  = 2,
    MFListAllocCateg_128K = 3,
    MFListAllocCateg_512K = 4,
    MFListAllocCateg_More = 5,

    MFListAllocCateg_Last,
};

static const U32 s_mflist_categ_sizes[] =
{
    KILOBYTES(16),
    KILOBYTES(32),
    KILOBYTES(128),
    KILOBYTES(1024),
    KILOBYTES(8192),

    /* Allocations of `MFListChunkSizeIndex_512K` or more size will have a dedicated mmap chunk */
    0,
};

static_assert(MFListAllocCateg_Last == ARRAY_LEN(s_mflist_categ_sizes), "");


ATTRIB_CONST static inline MFListBlock *
mflist__get_first_block_from_chunk(MFListChunk *chunk)
{
    MFListBlock *result = ( (MFListBlock *)
             (  (U8*) chunk + sizeof(MFListChunk))

        );

    internal_assert( chunk->size > (U8*) result - (U8*) chunk );

    result = (MFListBlock*) ALIGN(usize, result, sizeof(MFListBlock));

    return result;
}


ATTRIB_PURE static inline MFListBlock *
mflist__next_block(MFListChunk *chunk,
                   MFListBlock *block)
{
    MFListBlock *result = (MFListBlock *) (
        (U8*) block + sizeof(MFListBlock) + block->size
        );

    if ((U8*) result + sizeof(MFListBlock) < (U8*) chunk + chunk->size)
    {
        return result;
    }
    else
    {
        return NULL;
    }
}


#define MFLIST__ENABLE_DEBUG_INTEGRITY_CHECKS (1) && __DEBUG

#if MFLIST__ENABLE_DEBUG_INTEGRITY_CHECKS
#  define __mflist_assert_integrity(...) __mflist_assert_integrity__(__VA_ARGS__)
#else
#  define __mflist_assert_integrity(...)
#endif


#if MFLIST__ENABLE_DEBUG_INTEGRITY_CHECKS
void
__mflist_assert_integrity__(MFListChunk *chunk)
{
    size_t max_contiguous_block_size_avail = 0;
    MFListBlock *block = mflist__get_first_block_from_chunk(chunk);

    while(block)
    {

        internal_assert(block->parent_chunk == chunk);
        /* Assert block pointer lives inside the chunk region */

        internal_assert((U8*) block + sizeof (MFListBlock) <=
                        (U8*) chunk + chunk->size);

        /* The block should be at a multiple of the block size (16 bytes for 64 bits) */
        internal_assert((usize) block % sizeof(MFListBlock) == 0);

        if (block->is_avail && block->size > max_contiguous_block_size_avail)
        {
            max_contiguous_block_size_avail = block->size;
        }

        MFListBlock *temp = block;
        (void) temp;
        block = mflist__next_block(chunk, block);

        /* Assert the block is pointing */
        if (block)
        {
            internal_assert(block->prev_block == temp);
        }
    }

    internal_assert(max_contiguous_block_size_avail == chunk->max_contiguous_block_size_avail);
}
#endif

ATTRIB_PURE MFListBlock *
mflist__get_block_from_user_addr(void *_addr)
{
    MFListBlock *result = (MFListBlock *)
        ( (U8*)_addr - sizeof(MFListBlock));
    internal_assert(result == container_of(MFListBlock, _addr, payload));
    return result;
}




#if 0
ATTRIB_PURE static MFListUserAddrLocation
mflist__find_user_addr_location(MFList *mflist,
                                void *_addr)
{
    MFListUserAddrLocation result = { 0 };
    U8 *addr = _addr;

    /* Find The chunk first */
    for (enum MFListAllocCateg categ = 0;
         categ < ARRAY_LEN(mflist->chunks);
         categ ++ )
    {
        MFListChunk *prev_chunk = NULL;
        MFListChunk *chunk = mflist->chunks[categ];

        while(chunk)
        {
            __mflist_assert_integrity(chunk);


            if ((addr > ((U8*) chunk)) && (addr < (((U8*) chunk) + chunk->size)))
            {
                result.categ = categ;
                result.prev_chunk = prev_chunk;
                result.chunk = chunk;
                goto end_chunk_search;
            }

            prev_chunk = chunk;
            chunk = chunk->next_chunk;
        }
    }

end_chunk_search: { }

    /* Determine the correct block from the chunk */
    if (result.chunk)
    {
        MFListChunk *chunk = result.chunk;

        for (MFListBlock *block = mflist__get_first_block_from_chunk(result.chunk);
             block;
             block = mflist__next_block(chunk, block))
        {
            if ((U8*) block == (addr - sizeof(MFListBlock)))
            {
                result.block = block;
                break;
            }
        }
    }


    return result;
}

#endif


static inline void
mflist__init_chunk(MFListChunk *chunk, U32 chunk_size, enum MFListAllocCateg categ)
{
    chunk->prev_chunk = NULL;
    chunk->next_chunk = NULL;
    chunk->size = chunk_size;
    chunk->categ = categ;

    MFListBlock *first_block = mflist__get_first_block_from_chunk(chunk);
    {
        *first_block = (MFListBlock) {0};
        first_block->parent_chunk = chunk;
        first_block->prev_block = NULL;
        first_block->is_avail = true;
        first_block->size = chunk->size - (U32) sizeof(MFListChunk) - (U32) sizeof(MFListBlock);
    }

    chunk->max_contiguous_block_size_avail = first_block->size;
}

static inline MFListChunk *
mflist__new_chunk(U32 chunk_size, enum MFListAllocCateg categ)
{
    assert(IS_PAGE_ALIGNED(chunk_size));
    MFListChunk *newchunk = (MFListChunk*) mem_mmap(chunk_size);

    if (newchunk)
    {
        mflist__init_chunk(newchunk, chunk_size, categ);
    }

    return newchunk;
}



static inline MFListChunk *
mflist__chain_new_chunk(MFList *mflist,
                        MFListChunk *prev_chunk,
                        U32 chunk_size,
                        enum MFListAllocCateg categ)
{
    MFListChunk *newchunk = mflist__new_chunk(chunk_size, categ);

    if (newchunk)
    {
        newchunk->prev_chunk = prev_chunk;
        mflist->total_allocator_memory_usage += chunk_size;
    }

    if (prev_chunk)
    {
        prev_chunk->next_chunk = newchunk;
    }

    return newchunk;
}


static inline void
mflist__del_chunk(MFListChunk *chunk_to_be_deleted)
{
    mem_unmap(chunk_to_be_deleted, chunk_to_be_deleted->size);
}


static inline void
mflist__del_chained_chunk(MFList *mflist,
                          MFListChunk *chunk_to_be_deleted)
{
    MFListChunk *const prev_chunk = chunk_to_be_deleted->prev_chunk;
    const enum MFListAllocCateg categ = chunk_to_be_deleted->categ;

    if (prev_chunk)
    {
        prev_chunk->next_chunk = chunk_to_be_deleted->next_chunk;
    }
    else
    {
        internal_assert(categ >= 0 && categ < MFListAllocCateg_Last);
        mflist->chunks[categ] = chunk_to_be_deleted->next_chunk;
    }
    mflist->total_allocator_memory_usage -= chunk_to_be_deleted->size;
    mflist__del_chunk(chunk_to_be_deleted);
}




ATTRIB_PURE static inline bool
mflist__is_chunk_empty(MFListChunk *chunk)
{
    bool result = false;

    MFListBlock *first_block = mflist__get_first_block_from_chunk(chunk);
    MFListBlock *next_block = mflist__next_block(chunk, first_block);

    if (first_block && !first_block->is_avail && !next_block)
    {
        result = true;
        internal_assert(first_block->size == chunk->size - (U32) sizeof(MFListChunk));
    }

    return result;
}


/* Returns the merged blocks if they happened to merge with subsequent adjacent blocks
   or if no merge happened returns the same block passed (`loc->block`).
   @NOTE It may return NULL upon error or in case the blocks gets unmapped
   due to belonging to the categ of unbounded allocations (eg 1 dedicated chunk per allocation)*/
static MFListBlock *
mflist__free_block_and_merge(MFList *mflist,
                             MFListBlock *block_to_be_freed)
{
    assert_msg(block_to_be_freed, "Couldn't find the block requested\n Possible invalid pointer or the chunk got resized (NOTE :: Chunks are not allowed to resize by design : Possible memory corruption)");
    assert_msg(block_to_be_freed->parent_chunk, "Couldn't find the block requested\n Possible invalid pointer or the chunk got resized (NOTE :: Chunks are not allowed to resize by design : Possible memory corruption)");
    assert_msg(!block_to_be_freed->is_avail, "Possible Double free");

    __mflist_assert_integrity(block_to_be_freed->parent_chunk);

    MFListBlock *result = block_to_be_freed;

    if (!block_to_be_freed->parent_chunk || !(block_to_be_freed->is_avail))
    {
        return NULL;
    }
    MFListChunk *const parent_chunk = block_to_be_freed->parent_chunk;

    const enum MFListAllocCateg categ = (parent_chunk->categ);

    if (categ == MFListAllocCateg_More)
    {
        mflist__del_chained_chunk(mflist, parent_chunk);
        return NULL;
    }


    MFListBlock *block = block_to_be_freed;

    MFListBlock *prev_block = block->prev_block;
    MFListBlock *next_block = mflist__next_block(parent_chunk, block);

    const U32 block_size_before_dellocation = block->size;
    U32       combined_size                 = block->size;

    mflist->total_user_memory_usage -= block_size_before_dellocation;

    /* Try to combine blocks back together */
    if (prev_block && prev_block->is_avail)
    {
        if (next_block && next_block->is_avail)
        {
            combined_size = prev_block->size
                + (U32) sizeof(MFListBlock) + block->size
                + (U32) sizeof(MFListBlock) + next_block->size;
            prev_block->size = combined_size;
        }
        else
        {
            combined_size = prev_block->size + (U32) sizeof(MFListBlock) + block->size;
            prev_block->size = combined_size;
        }
        /* Set this new merged block as the new thing that needs to be returned */
        result = prev_block;
    }
    else
    {
        block->is_avail = true;


        /* Is it legit to assert on this or do we need to add the code to update
         the previous block :: `block->prev_block = prev_block;` ??? */
        internal_assert(block->prev_block == prev_block);

        if (next_block && next_block->is_avail)
        {
            combined_size = block->size + (U32) sizeof(MFListBlock) + next_block->size;
            block->size = combined_size;
        }
        else if (next_block)
        {

            /* Is it legit to assert on this or do we need to add the code to update
               the previous block :: `next_block->prev_block = block;` ??? */
            internal_assert(next_block->prev_block == block);
        }
        else
        {
            /* @NOTE :: INTENTIONALLY EMPTY */
        }
    }

    if (combined_size > parent_chunk->max_contiguous_block_size_avail)
    {
        parent_chunk->max_contiguous_block_size_avail = combined_size;
    }

    __mflist_assert_integrity(parent_chunk);


    return result;
}

void
mflist_free(MFList *mflist, void *_addr)
{
    MFListBlock *block_to_be_freed = mflist__get_block_from_user_addr(_addr);

    /* @NOTE Discard result, we don't care about it */
    (void) mflist__free_block_and_merge(mflist, block_to_be_freed);
}


ATTRIB_PURE static inline bool
mflist__should_del_chunk(MFListChunk *chunk)
{
    return ((chunk->size > KILOBYTES(64))
            || mflist__is_chunk_empty(chunk)
        );
}

void
mflist_clear(MFList *mflist)
{
    for (enum MFListAllocCateg categ = 0; categ < ARRAY_LEN(mflist->chunks); categ ++ )
    {
        MFListChunk *prev_chunk = NULL;
        MFListChunk *chunk = mflist->chunks[categ];

        if (!chunk)
        {
            continue;
        }

        MFListChunk *tmp = chunk->next_chunk;
        if (mflist__should_del_chunk(chunk))
        {
            mflist__del_chunk(chunk);
            chunk = prev_chunk;
        }
        else
        {
            mflist__init_chunk(chunk, chunk->size, chunk->categ);
        }

        prev_chunk = chunk;
        chunk = tmp;
        internal_assert(prev_chunk);

        while (chunk)
        {
            tmp = chunk->next_chunk;
            if (mflist__should_del_chunk(chunk))
            {
                mflist__del_chained_chunk(mflist, chunk);
                chunk = prev_chunk;
            }
            else
            {
                mflist__init_chunk(chunk, chunk->size, chunk->categ);
            }
            prev_chunk = chunk;
            chunk = tmp;
        }
    }
}


void
mflist_del(MFList *mflist)
{
    assert(mflist->chunks);

    for (enum MFListAllocCateg categ = 0; categ < ARRAY_LEN(mflist->chunks); categ ++)
    {
        MFListChunk *chunk = mflist->chunks[categ];
        if (!chunk)
        {
            continue;
        }


        while(chunk)
        {
            MFListChunk *tmp = chunk->next_chunk;
            mflist__del_chunk(chunk);
            chunk = tmp;
        }
    }

    memclr(mflist, sizeof(*mflist));
}

typedef struct MFListChunkInitRequirements
{
    enum MFListAllocCateg categ;
    U32                   required_chunk_size;
} MFListChunkInitRequirements;


static MFListChunkInitRequirements
mflist__get_chunk_requirements(MFList *mflist,
                               U32 alloc_size)
{
    const U32 needed_chunk_size = alloc_size + (U32) sizeof(MFListChunk) + (U32) sizeof(MFListBlock);

    MFListChunkInitRequirements result = {0};
    result.categ = MFListAllocCateg_More;
    result.required_chunk_size = (U32) PAGE_ALIGN(needed_chunk_size);


    /* Depending on how big the allocation is find the correct
       categ where to allocate. We use different
       chains to maintain different order of allocations sizes.
       In this way we can limit high fragmentation inside the chunk
       in case where big allocation and small allocations concur to
       use the same chunks */
    for (enum MFListAllocCateg i = 0; i < ARRAY_LEN(mflist->chunks); i++)
    {
        if (needed_chunk_size <= (U32) (0.25f * (float) s_mflist_categ_sizes[i]))
        {
            result.categ = i;
            result.required_chunk_size = s_mflist_categ_sizes[result.categ];
            break;
        }
    }

    return result;
}


void
mflist__suballoc_fork(MFList *mflist,
                      MFListChunk *allocatable_chunk,
                      MFListBlock *allocatable_block,
                      U32 alloc_size,
                      const bool zero_initialize)
{
    internal_assert((size_t) alloc_size % sizeof(MFListBlock) == 0);
    internal_assert(allocatable_block->size >= alloc_size);

    U32 remainder_size = allocatable_block->size - alloc_size;

    const enum MFListAllocCateg categ = allocatable_chunk->categ;
    const U32 chunk_size = allocatable_chunk->size;


    const bool should_fit_a_new_block =
        ((categ != MFListAllocCateg_More)
         && remainder_size > sizeof(MFListBlock)
         /* Avoid to create too small blocks, thus creating a long linked list
            of very very small blocks that are pretty much unusable */
         && (remainder_size > (U32) (0.1f * (float) chunk_size)));

    if (should_fit_a_new_block)
    {
        MFListBlock *newblock = (MFListBlock*) ((U8*) allocatable_block
                                   + sizeof(MFListBlock)
                                   + alloc_size);

        /* Would the neblow fit ? */
        if ((U8*) newblock + sizeof(MFListBlock) < (U8*)allocatable_chunk + allocatable_chunk->size)
        {
            newblock->parent_chunk = allocatable_chunk;
            newblock->prev_block = allocatable_block;
            newblock->size = ( remainder_size - (U32) sizeof(MFListBlock) );
            newblock->is_avail = true;
        }
    }
    else
    {
        /* Not enough space. Let the `allocatable_block`
           eat this little space cause we will not probably
           be able to fit anything in here */
        remainder_size = 0;
    }

    allocatable_block->is_avail = false;
    allocatable_block->size = allocatable_block->size - remainder_size;


    mflist->total_user_memory_usage += allocatable_block->size;

    if (zero_initialize)
    {
        memclr((U8*) allocatable_block + sizeof(MFListBlock), allocatable_block->size);
    }
}


void *
mflist_alloc1(MFList *mflist, U32 alloc_size, const bool zero_initialize)
{
    /* Always give a little bit of more room to avoid
       stupind off by 1 errors in case of string indexing for example
    */
    assert(alloc_size);
    if (alloc_size == 0)
    {
        return NULL;
    }
    alloc_size += (U32) sizeof(MFListBlock);
    alloc_size = ALIGN(U32, alloc_size, sizeof(MFListBlock));

    MFListChunkInitRequirements req = mflist__get_chunk_requirements(mflist, alloc_size);

    /* Allocate the first chunk if it's the first time */
    if (!mflist->chunks[req.categ])
    {
        mflist->chunks[req.categ] = mflist__chain_new_chunk(mflist,
                                                            NULL,
                                                            req.required_chunk_size,
                                                            req.categ);
    }

    if (mflist->chunks[req.categ] == NULL)
    {
        return NULL;
    }


    /* Now determine the correct chunk where we should allocate */
    MFListChunk *allocatable_chunk = NULL;
    {
        MFListChunk *chunk = mflist->chunks[req.categ];
        while (chunk)
        {
            __mflist_assert_integrity(chunk);
            if (chunk->max_contiguous_block_size_avail >= alloc_size)
            {
                allocatable_chunk = chunk;
                break;
            }
            else
            {
                /* Try to chain/append a new chunk in case we ran out */
                if (!chunk->next_chunk)
                {
                    chunk->next_chunk = mflist__chain_new_chunk(mflist, chunk, req.required_chunk_size, req.categ);
                    if (!chunk->next_chunk)
                    {
                        /* Failed */
                        break;
                    }
                    else
                    {
                        allocatable_chunk = chunk->next_chunk;
                        break;
                    }
                }
            }
            chunk = chunk->next_chunk;
        }
    }

    if (!allocatable_chunk)
    {
        return NULL;
    }

    __mflist_assert_integrity(allocatable_chunk);
    void *result = NULL;

    internal_assert(allocatable_chunk->max_contiguous_block_size_avail >= alloc_size);

    MFListBlock *allocatable_block               = NULL;

    /* Used to update the `allocatble_chunk->max_contiguous_block_size_avail`
       accordingly after a block allocation */
    U32          max_contiguous_block_size_avail = 0;


    /* Find an alloctable Block */
    MFListBlock *block = mflist__get_first_block_from_chunk(allocatable_chunk);
    {

        while(block)
        {
            if (block->is_avail && block->size >= alloc_size)
            {
                /* Found the block  */
                allocatable_block = block;
                break;
            }

            if (block->is_avail && block->size > max_contiguous_block_size_avail )
            {
                max_contiguous_block_size_avail = block->size;
            }

            block = mflist__next_block(allocatable_chunk, block);
        }
    }

    internal_assert(allocatable_block);
    internal_assert(allocatable_block->is_avail);

    /* Suballocate from the the found block */

    __mflist_assert_integrity(allocatable_chunk);
    mflist__suballoc_fork(mflist,
                          allocatable_chunk,
                          allocatable_block,
                          alloc_size,
                          zero_initialize);

    /* After the suballoc we're in a non
       valid integrity state. We need to keep
       to scan the entire blocks inside the chunk
       in order to update the `max_contiguos_block_size_avail`
       accordingly */
    if (0) { __mflist_assert_integrity(allocatable_chunk); }


    result = (U8*) allocatable_block + sizeof(MFListBlock);




    internal_assert(block == allocatable_block);

    /* Keep on looping the chain of blocks in order
       to properly update the `max_contiguos_block_size_avail` for the chunk */
    while(block)
    {
        if (block->is_avail && block->size > max_contiguous_block_size_avail)
        {
            max_contiguous_block_size_avail = block->size;
        }

        block = mflist__next_block(allocatable_chunk, block);
    }

    allocatable_chunk->max_contiguous_block_size_avail = max_contiguous_block_size_avail;

    __mflist_assert_integrity(allocatable_chunk);

    return result;
}


void *
mflist_realloc1 (MFList *mflist, void *oldptr, U32 newsize, bool zero_initialize)
{
    assert(newsize);
    if (newsize == 0)
    {
        return NULL;
    }
    newsize += (U32) sizeof(MFListBlock);
    newsize = ALIGN(U32, newsize, sizeof(MFListBlock));

    void *result = NULL;

    MFListBlock *rblock = mflist__get_block_from_user_addr(oldptr);
    MFListChunk *rchunk = rblock->parent_chunk;
    const enum MFListAllocCateg categ = rchunk->categ;

    assert_msg(rblock->size, "Corrupted Block. Possible Memory overflow or Undeflow in UserLand");
    assert_msg(rblock->is_avail == false, "Corrupted Block. Possible Memory overflow or Undeflow in UserLand");
    assert_msg(rchunk, "Corrupted Block. Possible Memory overflow or Undeflow in UserLand");

    if (rchunk && !(rblock->is_avail))
    {
        MFListChunkInitRequirements req = mflist__get_chunk_requirements(mflist, newsize);

        if ((categ == req.categ)
            && (rblock->size >= newsize))
        {
            /* We can reuse the same block, no need to free & alloc and copy data
               over */
            result = (U8*) (rblock) + sizeof(MFListBlock);

            if (zero_initialize)
            {
                memclr((U8*) result + newsize, rblock->size - newsize);
            }
        }
        else
        {
            /* @NOTE We can `free` prematurely the block. We're not going to
               lose the data that was there. Free is not a destructive operation.
               It can merge blocks together but it does not touch memory newmemory
               with new data, it just touches already existing block headers.
            */
            U8 *old_block_addr = (U8*) rblock;
            U32 old_block_size = rblock->size;
            (void) old_block_addr, (void) old_block_size;

            MFListBlock *merged_block = mflist__free_block_and_merge(mflist, rblock);
            (void) merged_block;

            result = mflist_alloc1(mflist, newsize, false);
            if (result)
            {
                MFListBlock *allocated_block = (MFListBlock*) ((U8*) result - sizeof(MFListBlock));

                /* No need to waste time copying data around if we get back the
                   same pointer (@NOTE Data is explicity not cleared upon free and alloc
                   in this case) */
                if (result != oldptr)
                {
                    internal_assert(!allocated_block->is_avail);
                    //internal_assert(newsize == allocated_block->size);

                    /* Here we can use memcpy instead of memmove. Memory regions
                       are not going to collide. Let's assert it*/
#if __DEBUG
                    /* Interval of the old allocation */
                    U8 *o1 = (U8*) old_block_addr + sizeof(MFListBlock);
                    U8 *o2 = (U8*) old_block_addr + sizeof(MFListBlock) + old_block_size;
                    /* Interval new allocation */
                    U8 *n1 = (U8*) result;
                    U8 *n2 = (U8*) result + newsize;

                    internal_assert(
                        ((o1 < n1) && (o2 < n1))
                        || ((o1 >= n2))
                        );
#endif
                    memcpy(result, oldptr, MIN(newsize, old_block_size));
                }

                if (zero_initialize)
                {
                    memclr((U8*) result + newsize, allocated_block->size - newsize);
                }
            }
        }
    }

    return result;
}




/* #############################################################################
   MArena Implementation
   #############################################################################
*/




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

    void *buffer = mem_realloc (arena->realloc_strategy,
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
        newsize = (U32) ((F32) arena->data_max_size * 1.25f) + 8 * (U32) G_pal.page_size;
    }
    return marena_realloc(arena, newsize);
}


#ifndef __DPCRT_MEM_LAYER__ARENA_ALWAYS_FORCE_REALLOC_AT_EVERY_PUSH
#  define __DPCRT_MEM_LAYER__ARENA_ALWAYS_FORCE_REALLOC_AT_EVERY_PUSH 1
#endif

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
    else if (__DPCRT_MEM_LAYER__ARENA_ALWAYS_FORCE_REALLOC_AT_EVERY_PUSH & can_realloc)
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
    size = ALIGN(U32, size, MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE);
    const U32 alignment = 128;
    size = ALIGN(U32, size, alignment);
    const size_t alloc_size = size;

    void *buffer = mem_alloc( alloc_strategy, alloc_size, alignment);

    if (buffer)
    {
        marena.buffer = buffer;
        // We're going to reserve the first 64 bytes
        // so we can return `MRef` (indices to the buffer)
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
marena_pop_upto(MArena *arena, MRef ref)
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
marena_fetch( MArena *arena, MRef ref, void *output, U32 sizeof_elem )
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


MRef
marena_commit (MArena *arena)
{
    MRef ref = 0;

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

    return marena_add_data(arena, pstr32, (U32) sizeof(Str32Hdr) + (U32) pstr32->len + U32_LIT(1));
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
    assert(str32.len >= 0);

    bool result = marena_add_data(arena, &str32, (U32) sizeof(Str32Hdr));
    if (result)
    {
        U8 *const string_data = (U8*) &str32 + sizeof(Str32Hdr);
        result = marena_add_data(arena, string_data, (U32) str32.len + U32_LIT(1));
    }
    return result;
}


bool
marena_ask_alignment(MArena *arena, U32 alignment)
{
    assert(arena && (arena->data_size != 0));
    const usize curr_addr = (usize) arena->buffer + arena->data_size;
    const usize aligned_addr = (usize) ALIGN(usize, curr_addr, alignment);
    const U32 required_bytes_for_alignment = (U32) (aligned_addr - curr_addr);
    return marena_add(arena, required_bytes_for_alignment, true);
}


#define MARENA_PUSH_WRAPPER_DEF(...)            \
    marena_begin(arena);                        \
    __VA_ARGS__;                                \
    return marena_commit(arena);






/* @NOTE :: Those functions bundles a `marena_begin()` `marena_commit()`
   calls for usage convenience */
MRef marena_push                (MArena *arena, U32 size, bool initialize_to_zero )	{ MARENA_PUSH_WRAPPER_DEF(marena_add                (arena, size, initialize_to_zero)) }
MRef marena_push_data           (MArena *arena, void *data, U32 sizeof_data )		{ MARENA_PUSH_WRAPPER_DEF(marena_add_data           (arena, data, sizeof_data)) }
MRef marena_push_pointer        (MArena *arena, void *pointer)				{ MARENA_PUSH_WRAPPER_DEF(marena_add_pointer        (arena, pointer)) }
MRef marena_push_byte           (MArena *arena, byte_t b )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_byte           (arena, b )) }
MRef marena_push_char           (MArena *arena, char c )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_char           (arena, c )) }
MRef marena_push_i8             (MArena *arena, I8 i8 )					{ MARENA_PUSH_WRAPPER_DEF(marena_add_i8             (arena, i8 )) }
MRef marena_push_u8             (MArena *arena, U8 u8 )					{ MARENA_PUSH_WRAPPER_DEF(marena_add_u8             (arena, u8 )) }
MRef marena_push_i16            (MArena *arena, I16 i16 )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_i16            (arena, i16 )) }
MRef marena_push_u16            (MArena *arena, U16 u16 )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_u16            (arena, u16 )) }
MRef marena_push_i32            (MArena *arena, I32 i32 )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_i32            (arena, i32 )) }
MRef marena_push_u32            (MArena *arena, U32 u32 )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_u32            (arena, u32 )) }
MRef marena_push_i64            (MArena *arena, I64 i64 )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_i64            (arena, i64 )) }
MRef marena_push_u64            (MArena *arena, U64 u64 )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_u64            (arena, u64 )) }
MRef marena_push_size_t         (MArena *arena, size_t s )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_size_t         (arena, s )) }
MRef marena_push_usize          (MArena *arena, usize us )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_usize          (arena, us )) }
MRef marena_push_cstr           (MArena *arena, char* cstr )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_cstr           (arena, cstr )) }
MRef marena_push_pstr32         (MArena *arena, PStr32 *pstr32 )			{ MARENA_PUSH_WRAPPER_DEF(marena_add_pstr32        (arena, pstr32 )) }
MRef marena_push_str32_nodata   (MArena *arena, Str32 str32 )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_str32_nodata   (arena, str32 )) }
MRef marena_push_str32_withdata (MArena *arena, Str32 str32 )				{ MARENA_PUSH_WRAPPER_DEF(marena_add_str32_withdata (arena, str32 )) }
MRef marena_push_alignment      (MArena *arena, U32 alignment)				{ MARENA_PUSH_WRAPPER_DEF(marena_ask_alignment      (arena, alignment)) }
