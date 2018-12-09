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

#ifndef HGUARD_77f71620e38a4ba09d5a55b0bc707a92
#define HGUARD_77f71620e38a4ba09d5a55b0bc707a92

#include "dpcrt_utils.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "dpcrt_types.h"
#include "dpcrt_pal.h"


__BEGIN_DECLS

enum AllocStrategy {
    AllocStrategy_Xmalloc       = 0,
    AllocStrategy_Malloc        = 1,
    AllocStrategy_Xcalloc       = 2,
    AllocStrategy_Calloc        = 3,
    AllocStrategy_AlignedAlloc  = 4,
    AllocStrategy_Mmap          = 5,
    AllocStrategy_Sbrk          = 6,          // @NOTE: Reallocation is supported only if it is the last allocated in a LIFO style
};



enum ReallocStrategy {
    ReallocStrategy_None            = 0,
    ReallocStrategy_Xrealloc        = 1,
    ReallocStrategy_Realloc         = 2,
    ReallocStrategy_MRemap_MayMove  = 3, // Pointers inside the buffer may get invalidated. May need to use offset
                                         // inside the buffer which are relative position independent from the start
                                         // of the buffer.
    ReallocStrategy_MRemap_KeepAddr = 4, // Try to keep the address the same. Pointers to the memory region
                                         // will not get invalidate. May fail more easily if the OS could not fit
                                         // the memory region in the same virtual address space
};

enum DeallocStrategy {
    DeallocStrategy_Free = 0,
    DeallocStrategy_Munmap = 1,
};




// @IMPORTANT @NOTE: a MemRef with a 0 rel_offset value should be considered invalid and not pointing
//  to something usefull. Implementations of memory allocators making use of this
//  `MemRef` should skip 1 byte after the creation in order to reserve the zero-eth offset.
//   Usually a MemRef is implemented with a index (relative offset)
typedef U32 mem_ref_t;

void* mem_mmap(size_t size);
void  mem_unmap(void *addr, size_t size);

void*
mem_alloc( enum AllocStrategy alloc_strategy,
           size_t size,
           size_t alignment );

void*
mem_realloc__release ( enum ReallocStrategy realloc_strategy,
                       void  *old_addr,
                       size_t old_size,
                       size_t new_size,
                       size_t alignment );

#if MEMORY_LAYER_DEBUG_CODE
void*
mem_realloc__debug ( enum ReallocStrategy realloc_strategy,
                     void  *old_addr,
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
mem_realloc ( ReallocStrategy realloc_strategy,
              void  *addr,
              size_t old_size,
              size_t new_size,
              size_t alignment);
#endif

void
mem_dealloc (enum DeallocStrategy dealloc_strategy,
             void *addr,
             size_t  buffer_size);


/* @TAG: Generic Allocation, malloc, realloc, free */

void*
xmalloc (size_t size);

void*
xrealloc (void *ptr, size_t newsize );




__END_DECLS

#endif  /* HGUARD_77f71620e38a4ba09d5a55b0bc707a92 */
