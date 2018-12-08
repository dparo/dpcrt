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

#ifndef HGUARD_6cae59f8ded7434090c01c15fe03a866
#define HGUARD_6cae59f8ded7434090c01c15fe03a866

#include "dpcrt_types.h"
#include "dpcrt_utils.h"


__BEGIN_DECLS


typedef struct AllocationChunk
{
    /* Size of the chunk that was allocated successfully.
       In case the allocation failed, this field _should_
       be set to 0, to avoid usage of uninitialized memory. */
    size_t  size_of_chunk;

    /* Pointer to the actual allocated block of memory.
       It may be `NULL` in case the allocation failed */
    void    *data;
} AllocationChunk;



typedef struct FreeListBlock
{
    /* Flag marking if ALL the following blocks up to the end of the
       chunk are freed independently of the value assumed from `next_block` field.
       This flag is not strictly necessary to make the freelist working but is
       instead an optimization. It allows when `clearing` an entire
       chunk to avoid scanning the entire said chunk to set up all the linked list of blocks
       pointer, instead we clear only the first block and set the subsequent
       `following_blocks_are_all_free` field to true. */
    bool32 following_blocks_are_all_free;
    struct FreeListBlock *next_block;
    /* ... PAYLOAD .... */
} FreeListBlock;

typedef struct FreeListChunk
{
    struct FreeListChunk *next_chunk;
    struct FreeListBlock *next_block;

    /* ---- */
    U8 payload[];
} FreeListChunk;

typedef struct FreeList
{
    U32   chunk_size;
    U16   block_size;
    bool8 allocate_more_chunks_on_demand;
    
    FreeListChunk *first_chunk;
} FreeList;



/* Simple FreeList Allocator
   The block size determines the maximum possible
   allocatable size ( a high block_size value may
   lead to high internal memory fragmentation ).
   ---
   EXAMPLE:
   ---
   {
       FreeList freelist;
       freelist_init_aux(&freelist, (U32) KILOBYTES(64), 512, true); // Inits the arena asking for the OS to map memory. Each block will be able to store only 512 bytes or less
       void *p = freelist_alloc(freelist, 64);           // Since 64 bytes fits in 512 bytes the allocator will align the size
                                                         //    to 512 bytes and return you a block of 512 bytes
       void *q = freelist_alloc(freelist, 1024);         // returns NULL, the allocator cannot fit 1024 bytes of data
       freelist_free(freelist, p);                       // frees the block associated to pointer q
       freelist_clear(freelist);                         // frees EVERY BLOCK, but the actual memory is not unmapped, and it will be reused in subsequent `_alloc` calls
       freelist_del(freelist);                           // Invalidates the freelist, unmaps the memory letting the OS reclaim it
   }
*/

/* @NOTE :: `allocate_more_chunks_on_demand` allows the arena to try ask the OS to map more memory
   if it is required in order to fit a new allocation request. */
bool  freelist_init_aux (FreeList *freelist, U32 chunk_size, U16 block_size, bool8 allocate_more_chunks_on_demand );
bool  freelist_init     (FreeList *freelist, U16 block_size);
void* freelist_alloc    (FreeList *freelist, U16 size);
void  freelist_free     (FreeList *freelist, void *ptr);
void  freelist_clear    (FreeList *freelist);
void  freelist_del      (FreeList *freelist);


__END_DECLS

#endif /* HGUARD_6cae59f8ded7434090c01c15fe03a866 */
