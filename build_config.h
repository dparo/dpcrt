/*
 * Copyright (C) 2018  Davide Paro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

#include "compiler.h"

__BEGIN_DECLS

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
#  define MEMORY_LAYER_DEBUG_CODE ((1) && __DEBUG)
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
#  define MEMORY_ARENA_ALWAYS_FORCE_REALLOC_AT_EVERY_PUSH ((1) && __DEBUG)
#endif


/* =======================================
   LEXER CONFIGS 
   =======================================*/

/* Should the lexer generate new tokens for whitespaces,
   or simply "eat" them without generating tokens.
   @NOTE: Enabling this flag may require a more difficult
   parser implementation since it must be aware that 
   whitespaces between words and identifier maybe present or not,
   depending on the input*/
#ifndef LEX_WHITESPACES_ENABLED
#  define LEX_WHITESPACES_ENABLED (0)
#endif


__END_DECLS

#endif
