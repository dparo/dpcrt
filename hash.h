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

#ifndef HGUARD_27b30011b22943b69d0a4062f38d2698
#define HGUARD_27b30011b22943b69d0a4062f38d2698

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

#endif  /* HGUARD_27b30011b22943b69d0a4062f38d2698 */
