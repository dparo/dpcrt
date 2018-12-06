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
#ifndef HGUARD_5dc6b1c89a724ccfa1561f08d7344951
#define HGUARD_5dc6b1c89a724ccfa1561f08d7344951

/* =======================================
   UTILS AND PLATFORM LAYER CONFIGS 
   =======================================*/

/* Enable the memory layer debug code. The memory layer debug
   code will enable wherever possibile a different reallocation strategy.
   At every reallocation, the old buffer instead of being reallocated is
   marked with a mprotect() syscall and a new memory region is allocated instead.
   This allows to more easily catch pointers pointing to old memory regions
   in debug mode. Watch out. On some applications this can lead to huge
   memory consumption depending on the amount of allocation requested from 
   the applications */
// Enable the memory layer debug code
#ifndef MEMORY_LAYER_DEBUG_CODE
#  define MEMORY_LAYER_DEBUG_CODE ((0) && __DEBUG)
#endif

/*
   Generates huge memory consumption, if used when `MEMORY_ARENA_ALWAYS_FORCE_REALLOC_AT_EVERY_PUSH` is set !!!
   Maybe in the future we will allow you to set this variable
   at runtime to check subportion of the suspect code
*/
#ifndef MEMORY_LAYER_MREMAP_PROTECT_OLD_MMAPED_REGION
#  define MEMORY_LAYER_MREMAP_PROTECT_OLD_MMAPED_REGION ((0) && __DEBUG)
#endif

/* Forces a reallocation at every push even if it is not necessary.
   If you use this with the `MEMORY_LAYER_DEBUG_CODE` set it will force
   the memory to change address, so you can potentially find addresses
   that are not ready to conform to a marena growing at any moment.
   Remember to store `mem_ref_t` and not raw pointers ! */
#ifndef MEMORY_ARENA_ALWAYS_FORCE_REALLOC_AT_EVERY_PUSH
#  define MEMORY_ARENA_ALWAYS_FORCE_REALLOC_AT_EVERY_PUSH ((0) && __DEBUG)
#endif

#endif  /* HGUARD_5dc6b1c89a724ccfa1561f08d7344951 */
