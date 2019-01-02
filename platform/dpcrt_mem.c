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

#include "dpcrt_mem.h"
#include <stdc/malloc.h>
#include <stdc/stdio.h>



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


void*
mem_mmap(size_t size)
{
    void *addr = 0;
    const enum page_prot_flags prot = PAGE_PROT_READ | PAGE_PROT_WRITE;
    const enum page_type_flags type = PAGE_PRIVATE | PAGE_ANONYMOUS;
    size = PAGE_ALIGN(size);
    return pal_mmap_memory( addr, size, prot, type );
}


void
mem_unmap(void *addr, size_t size)
{
    assert(size == PAGE_ALIGN(size));
    bool result = pal_munmap(addr, size);
    (void) result;
    assert(result);
}

void*
mem_alloc( enum AllocStrategy alloc_strategy, size_t size, size_t alignment)
{
    (void) alignment;
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

#if 0
    case AllocStrategy_AlignedAlloc: {
        return aligned_alloc(alignment, size);
    } break;
#endif

    case AllocStrategy_Mmap: {
        return mem_mmap(size);
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
mem_realloc__release ( enum ReallocStrategy realloc_strategy,
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
mem_realloc__debug ( enum ReallocStrategy realloc_strategy,
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
        const enum AllocStrategy alloc_strategy = AllocStrategy_Mmap;
        void *new_addr = mem_alloc(alloc_strategy, new_size, alignment);
        memcpy(new_addr, old_addr, old_size);
#if MEMORY_LAYER_MREMAP_PROTECT_OLD_MMAPED_REGION
        const bool was_protected = pal_mprotect(old_addr, old_size, PAGE_PROT_NONE);
        assert(was_protected); // debug code we can simply assert here
#else
        mem_dealloc(DeallocStrategy_Munmap, old_addr, old_size);
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
mem_dealloc (enum DeallocStrategy dealloc_strategy,
             void *addr,
             size_t  size)
{
    switch (dealloc_strategy)
    {

    default: { invalid_code_path(); } break;

    case DeallocStrategy_Free: {
        free(addr);
    } break;

    case DeallocStrategy_Munmap: {
        mem_unmap(addr, size);
    } break;

    }
}


