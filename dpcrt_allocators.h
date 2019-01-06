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
#include "dpcrt_mem.h"


__BEGIN_DECLS

#if 0
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
#endif


typedef struct MPoolBlock
{
    /* Flag marking if ALL the following blocks up to the end of the
       chunk are freed independently of the value assumed from `next_block` field.
       This flag is not strictly necessary to make the mpool working but is
       instead an optimization. It allows when `clearing` an entire
       chunk to avoid scanning the entire said chunk to set up all the linked list of blocks
       pointer, instead we clear only the first block and set the subsequent
       `following_blocks_are_all_free` field to true. */
    bool32 following_blocks_are_all_free;
    struct MPoolBlock *next_block;
    /* ... PAYLOAD .... */
} MPoolBlock;

typedef struct MPoolChunk
{
    struct MPoolChunk *next_chunk;
    struct MPoolBlock *next_block;

    /* ---- */
    U8 payload[];
} MPoolChunk;



typedef struct MPool
{
    U32   chunk_size;
    U16   block_size;
    bool8 allocate_more_chunks_on_demand;

    size_t total_allocator_memory_usage;
    size_t total_user_memory_usage;

    MPoolChunk *first_chunk;
} MPool;



/* Simple MPool Allocator
   The block size determines the maximum possible
   allocatable size ( a high block_size value may
   lead to high internal memory fragmentation ).
   ---
   EXAMPLE:
   ---
   {
       MPool mpool;
       mpool_init_aux(&mpool, (U32) KILOBYTES(64), 512, true); // Inits the arena asking for the OS to map memory. Each block will be able to store only 512 bytes or less
       void *p = mpool_alloc(mpool, 64);           // Since 64 bytes fits in 512 bytes the allocator will align the size
                                                         //    to 512 bytes and return you a block of 512 bytes
       void *q = mpool_alloc(mpool, 1024);         // returns NULL, the allocator cannot fit 1024 bytes of data
       mpool_free(mpool, p);                       // frees the block associated to pointer q
       mpool_clear(mpool);                         // frees EVERY BLOCK, but the actual memory is not unmapped, and it will be reused in subsequent `_alloc` calls
       mpool_del(mpool);                           // Invalidates the mpool, unmaps the memory letting the OS reclaim it
   }
*/

/* @NOTE :: `allocate_more_chunks_on_demand` allows the arena to try ask the OS to map more memory
   if it is required in order to fit a new allocation request. */
bool  mpool_init_aux (MPool *mpool, U32 chunk_size, U16 block_size, bool8 allocate_more_chunks_on_demand );
bool  mpool_init     (MPool *mpool, U16 block_size);
void* mpool_alloc    (MPool *mpool, U16 size);
void  mpool_free     (MPool *mpool, void *ptr);
void  mpool_clear    (MPool *mpool);
void  mpool_del      (MPool *mpool);



struct MFListChunk;

/* Blocks are chained in sequential order inside a given chunk:
   As long we are inside the chunk region the next block is given by:

       next_block = (U8*) block + sizeof(MFListBlock) + block->size

   The previous block can simply be found by reading:

       block->prev_block

   which could be NULL in case the given block is the first in the chain.
*/
typedef struct MFListBlock
{
    struct MFListChunk *parent_chunk;
    struct MFListBlock *prev_block;
    U32    size;                /* Size of the payload available for USER ALLOCATION !
                                   The size of the entire block it's thus composed
                                   of `sizeof(MFListBlock) + block->size` */
    bool32 is_avail;
    /* ---- */
    /* U8 payload[]; */
} MFListBlock;

typedef struct MFListChunk
{
    
    /* Chain of chunks belonging to the same allocation categ.
       Or NULL in case it's the last one in the chain. */
    struct MFListChunk *prev_chunk;
    struct MFListChunk *next_chunk;

    /* Size of the entire chunk including: this header and the
       total payload length available. The payload length is made of the sum of:
       - All the Blocks headers
       - All user allocated payload data
    */
    U32   size;

    /* Optimization field. Every chunk stores the maximum size
       achieved by one of it's blocks. We can use this in the allocation
       phase. If the chunk doesn't have a `max_contiguous_block_size_avail`
       satisfying the requirement, we don't even bother to scan it's
       internal blocks and we pass directly to the next one.
       @NOTE :: A value of `0` for this field denotes that the chunk
       is full and there's no available block for allocation. */
    U32 max_contiguous_block_size_avail;
    U8 categ;
    /* ---- */
    /* U8 payload[]; */
} MFListChunk;



/* @NOTE :: The `mflist` uses internal assertion for
   verifying correctness. If an internal
   assertion triggers it may mean 2 things:
   - 1: Bug in the `mflist` implementation (very unlikely with more time this thing gets used)
   - 2: Memory corruption due to buffer overflow usage of the allocated memory. Those
        will very likely trigger `internal_assert` (if those are enabled) that
        checks for consistency of the state of `MFListBlock` 's.
 */
typedef struct MFList
{
    size_t total_allocator_memory_usage;
    size_t total_user_memory_usage;

    /* We use 6 different ""categories"" of allocation depending on the degree
       of the size of the allocations.
       - index 0: Contains usually small allocations on the average of 128~256 bytes
       - index 1: Contains small to medium allocations on the average of 1~2 Kilo
       - ...
       - index 6: Contains unbounded allocations that do not fit in the previous categories (More than 512K).
                  These allocations gets dedicated new chunks per allocation (eg 1 mmap per allocation) */
    MFListChunk *chunks[6];
} MFList;




/* A zero-initialization of an MFList is more than enough to have a valid MFList state.
   The function `mflist_init` is provided just for consistency reasons.

   @EXAMPLE USAGE:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   MFList mflist = {0};    // or equivalently mflist_init(&mflist);
   char *buf = mflist_alloc(&mflist, 16); // or `mflist_alloc1(&mflist, 16, true);` for explicitly requesting a clear of the returned buffer
   if (buf)
   {
      strncpy(buf, "Hello world", 16);
      printf("%s\n", buf);
      mflist_free(&mflist, buf);
   }
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


   Since zero initialization is more than enough for a correct initialization
   the MFList can be conveniently used even when declared in global scope (or static to the
   translation unit) and can just be used right away.

   @EXAMPLE:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   ....
   static MFList strings_allocator;

   // Wrapper functions for conveniently using the allocator
   char* new_string(char *s)
   {
       char *result = mflist_alloc1(&strings_allocator, (U32) strlen(s), false);
       if (result)
       {
           strcpy(result, s);
       }
       return result;
   }
   char* concat_strings(char *s1, char *s2)
   {
      U32 s1_len = (U32) strlen(s1);
      U32 s2_len = (U32) strlen(s2);
      char *result = mflist_alloc1(&strings_allocator, s1_len + s2_len, false);
      if (result)
      {
           strcpy(result, s1);
           strcpy(result + s1_len, s2);
      }
      return result;
   }
   ....
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 */
static inline bool mflist_init(MFList *mflist) { memclr(mflist, sizeof(MFList)); return true; }
bool  mflist_init     (MFList *mflist);
void* mflist_alloc1   (MFList *mflist, U32 alloc_size, bool zero_initialize);
void  mflist_free     (MFList *mflist, void *ptr);
void* mflist_realloc1 (MFList *mflist, void *oldptr, U32 newsize, bool zero_initialize);
void  mflist_clear    (MFList *mflist);
void  mflist_del      (MFList *mflist);
static inline void* mflist_alloc    (MFList *mflist, U32 size) { return mflist_alloc1(mflist, size, true); }
static inline void* mflist_realloc  (MFList *mflist, void *ptr, U32 newsize) { return mflist_realloc1(mflist, ptr, newsize, true); }










// @IMPORTANT @NOTE: a MRef with a 0 rel_offset value should be considered invalid and not pointing
//  to something usefull. Implementations of memory allocators making use of this
//  `MRef` should skip 1 byte after the creation in order to reserve the zero-eth offset.
//   Usually a MRef is implemented with a index (relative offset)
typedef U32 MRef;



typedef struct MArenaAtomicAllocationContext
{
    bool32 failed;
    U32    staging_size;
} MArenaAtomicAllocationContext;

#define MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE (16)

typedef struct MArena {
    /* For internal usage only */
    MArenaAtomicAllocationContext alloc_context;
    enum AllocStrategy   alloc_strategy;
    enum ReallocStrategy realloc_strategy;
    enum DeallocStrategy dealloc_strategy;
    /* ------------------- */

    U32                   data_size;
    U32                   data_max_size;

    U8* buffer;
} MArena;


MArena           marena_new_aux        ( enum AllocStrategy alloc_strategy,
                                         enum ReallocStrategy realloc_strategy,
                                         enum DeallocStrategy dealloc_strategy,
                                         U32 size );
MArena           marena_new            ( U32 size, bool may_grow );
void             marena_del            ( MArena *arena );

void             marena_pop_upto       ( MArena *arena, MRef ref );
void             marena_fetch          ( MArena *arena, MRef ref, void *output, U32 sizeof_elem );
void             marena_clear          ( MArena *arena );


/* Beging an atomic allocation context, you can start building up data incrementally
   directly on the arena. When calling `marena_commit` the data built up to that
   moment if there wasn't any error is going to be commited updating the `stack_pointer`
   and making the data actually ""visible"" to the user by returning a valid `MRef` to
   it.
   If you want to abort an atomic allocation context call `marena_dismiss`
   The `marena_add_xxxx` functionality allows you to construct data incrementally. They
   all return a bool saying if the request successed. You can choose to handle
   the failure right away by calling `marena_dismiss`, or just pretend
   nothing happened and keep pushing to it, once you will call `marena_commit`
   the function will return you a `MRef = 0` since one of the allocation failed.

   Between `marena_add_xxx` calls no alignment will be performed from the stack allocator,
   if you want alignment for performance reasons you must ask it explicitly.
*/
void             marena_begin              (MArena *arena);

bool             marena_add                (MArena *arena, U32 sizeof_data, bool initialize_to_zero );
bool             marena_add_data           (MArena *arena, void *data, U32 sizeof_data );
bool             marena_add_pointer        (MArena *arena, void *pointer);
bool             marena_add_byte           (MArena *arena, byte_t b );
bool             marena_add_char           (MArena *arena, char c );
bool             marena_add_i8             (MArena *arena, I8 i8 );
bool             marena_add_u8             (MArena *arena, U8 u8 );
bool             marena_add_i16            (MArena *arena, I16 i16 );
bool             marena_add_u16            (MArena *arena, U16 u16 );
bool             marena_add_i32            (MArena *arena, I32 i32 );
bool             marena_add_u32            (MArena *arena, U32 u32 );
bool             marena_add_i64            (MArena *arena, I64 i64 );
bool             marena_add_u64            (MArena *arena, U64 u64 );
bool             marena_add_size_t         (MArena *arena, size_t s );
bool             marena_add_usize          (MArena *arena, usize us );
bool             marena_add_cstr           (MArena *arena, char* cstr );
bool             marena_add_pstr32         (MArena *arena, PStr32 *pstr32 );
bool             marena_add_str32_nodata   (MArena *arena, Str32 str32 );
bool             marena_add_str32_withdata (MArena *arena, Str32 str32 );
bool             marena_ask_alignment      (MArena *arena, U32 alignment);

void             marena_dismiss            (MArena *arena);
MRef             marena_commit             (MArena *arena);





MRef marena_push                (MArena *arena, U32 sizeof_data, bool initialize_to_zero );
MRef marena_push_data           (MArena *arena, void *data, U32 sizeof_data );
MRef marena_push_pointer        (MArena *arena, void *pointer);
MRef marena_push_byte           (MArena *arena, byte_t b );
MRef marena_push_char           (MArena *arena, char c );
MRef marena_push_i8             (MArena *arena, I8 i8 );
MRef marena_push_u8             (MArena *arena, U8 u8 );
MRef marena_push_i16            (MArena *arena, I16 i16 );
MRef marena_push_u16            (MArena *arena, U16 u16 );
MRef marena_push_i32            (MArena *arena, I32 i32 );
MRef marena_push_u32            (MArena *arena, U32 u32 );
MRef marena_push_i64            (MArena *arena, I64 i64 );
MRef marena_push_u64            (MArena *arena, U64 u64 );
MRef marena_push_size_t         (MArena *arena, size_t s );
MRef marena_push_usize          (MArena *arena, usize us );
MRef marena_push_cstr           (MArena *arena, char* cstr );
MRef marena_push_pstr32         (MArena *arena, PStr32 *pstr32 );
MRef marena_push_str32_nodata   (MArena *arena, Str32 str32 );
MRef marena_push_str32_withdata (MArena *arena, Str32 str32 );
MRef marena_push_alignment      (MArena *arena, U32 alignment);







static inline void *
marena_unpack_ref__unsafe(MArena *arena, MRef ref)
{
    assert(arena->buffer);
    assert(arena->data_size);
    assert(ref && ref >= MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE && ref < arena->data_size);


    {
        /* @NOTE(dparo) [Mon Nov 26 22:05:28 CET 2018]

           It is not very polite to ask to access a raw pointer
           while in the middle of a `marena_begin` call.
           Accessing a pointer in the middle of `marena_begin` `marena_commit`
           pair can potentially be very unsafe due to the fact that the stack
           is not guaranteed to maintain the same address due to the fact
           that it may grow.
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

#endif /* HGUARD_6cae59f8ded7434090c01c15fe03a866 */
