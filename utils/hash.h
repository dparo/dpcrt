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
#ifndef HASH_H
#define HASH_H

#include "utils.h"
#include "types.h"

__BEGIN_DECLS


// @NOTE :: For reference on more hash functions:
//          https://en.wikipedia.org/wiki/List_of_hash_functions

static inline uint32 simple_hash(ptr_t buffer, size_t len)
{
    uint32 hash = 0;
    while (len-- > 0)
    {
        hash = (hash << 7) + (hash >> 25) + *buffer++;
    }
    return hash + (hash >> 16);
}

// Computes the Adler32  Checksum algorithm.
//   Can be used as a checksum or as a hash_function
//    to index on 32 bit sized hashmaps
// @NOTE :: https://en.wikipedia.org/wiki/Adler-32
static inline uint32 adler32(ptr_t buffer, const size_t len)
{
#define MOD_ADLER ((uint32) 65521)
    uint32 a = 1, b = 0;
    size_t index;


    // @NOTE :: Can be made much more efficient by using SIMD,
    //   or simply by pulling the buffer with a `size_t*` and
    //   then handling the remainder
    // Process each byte of the buffer in order
    for (index = 0; index < len; ++index)
    {
        a = (a + buffer[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }

    return (b << 16) | a;
#undef MOD_ADLER
}

__END_DECLS

#endif
