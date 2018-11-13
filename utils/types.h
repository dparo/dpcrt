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
#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "compiler.h"

__BEGIN_DECLS

typedef uint8_t* ptr_t;
typedef float    float32;
typedef double   float64;
typedef int8_t   int8;
typedef uint8_t  uint8;
typedef uint8    byte_t;
typedef int16_t  int16;
typedef uint16_t uint16;
typedef int32_t  int32;
typedef uint32_t uint32;
typedef int64_t  int64;
typedef uint64_t uint64;


typedef uint8_t* ptr_t;
typedef float    F32;
typedef double   F64;
typedef int8_t   I8;
typedef uint8_t  U8;
typedef uint8    Byte;
typedef int16_t  I16;
typedef uint16_t U16;
typedef int32_t  I32;
typedef uint32_t U32;
typedef int64_t  I64;
typedef uint64_t U64;



typedef struct Str32
{
    U32 len;
    char *data;
} Str32;

#define STR32_LIT(S)                            \
    (Str32) {                                   \
        ARRAY_LEN(S),                           \
            (char*) S                           \
            }



#if __PAL_LINUX__
#  if __PAL_ARCHITECTURE_SIZE__ == 64
typedef long int time_t;
#  else
#    error "Double check what's the correct time_t rappresentation for 32 bit machines."
#  endif
#endif

__END_DECLS

#endif
