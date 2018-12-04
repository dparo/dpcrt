#include "dpcrt_allocators.h"
#include "dpcrt_mem.h"




/* ##########################################################################
   FreeList Implementation
   ########################################################################## */

#define FREELIST_RESERVED_CHUNK_HEADER_SIZE ( ALIGN(sizeof(FreeListChunk), 128) )

static inline void
freelist_init_chunk(FreeListChunk *chunk)
{
    chunk->is_filling_up = true;
    chunk->next_chunk    = NULL;
    chunk->next_block    = (FreeListBlock*) ((U8*) chunk
                                             + FREELIST_RESERVED_CHUNK_HEADER_SIZE);
}

static FreeListChunk *
freelist_chain_new_chunk(U32 chunk_size)
{
    assert((chunk_size % PAGE_SIZE) == 0);
    FreeListChunk *chunk = (FreeListChunk*) mem_mmap(chunk_size);

    if (chunk)
    {
        freelist_init_chunk(chunk);
    }

    return chunk;
}


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


void
freelist_free(FreeList *freelist, void *addr)
{
    FreeListChunk *chunk = freelist_get_chunk_from_addr(freelist, addr);
    if (chunk)
    {
        chunk->is_filling_up = false;

        FreeListBlock *temp = chunk->next_block;
        chunk->next_block = (FreeListBlock *) addr;
        ((FreeListBlock *) addr)->next_block = temp;
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
    void *result = NULL;
    if (size > freelist->block_size)
    {
        return NULL;
    }

    assert(freelist->first_chunk);
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
                FreeListChunk *newchunk = freelist_chain_new_chunk(freelist->chunk_size);

                if (!newchunk)
                {
                    return NULL;
                }
                
                chunk->next_chunk = newchunk;
                chunk = newchunk;
            }
        }
    }


    /* Now we have a valid allocated chunk, let's use it for the allocation */
    {
        assert(chunk);
        assert(chunk->next_block);

        result = (void*) chunk->next_block;

        if (chunk->is_filling_up)
        {
            chunk->next_block = (FreeListBlock*) ((U8*) chunk->next_block + freelist->block_size);

            /* Can we actually fit an entire chunk size ?? If not, the chunk is now full */
            if ( ((U8*) chunk->next_block + freelist->block_size)
                 > ((U8*) chunk + freelist->chunk_size))
            {
                chunk->next_block = NULL;
            }
        }
        else
        {
            memclr(result, freelist->block_size);
            chunk->next_block = (FreeListBlock *) (*((void**) chunk->next_block));
        }

    }

    assert(result);

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


    freelist->first_chunk                    = freelist_chain_new_chunk(chunk_size);

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
