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

#ifndef HGUARD_eeac0bd03e2846dca1cd3fb01fa7efe1
#define HGUARD_eeac0bd03e2846dca1cd3fb01fa7efe1


#include <stddef.h>
#include <stdint.h>


__BEGIN_DECLS


typedef uint32_t packed_bool;


#define BOOL_PACKED_ARRAY_NELEMS(bitscount, typeof_arraymember)  \
    ((((bitscount) - 1) / (sizeof(typeof_arraymember) * 8)) + 1)

#define BOOL_PACKED_ARRAY_SIZE(bitscount, typeof_arraymember)           \
    BOOL_PACKED_ARRAY_NELEMS(bitscount, typeof_arraymember) * sizeof(typeof_arraymember)

#define BOOL_PACK_INTO_ARRAY(val, bit_index, array, array_num_members, typeof_arraymember) \
    do {                                                                \
        assert(bit_index < (array_num_members) * sizeof(typeof_arraymember) * 8); \
        size_t ___index___ = ((size_t)bit_index) / ( sizeof(typeof_arraymember) * 8); \
        (array)[___index___] =                                          \
            (typeof_arraymember)((array)[___index___]                   \
                                 & ~((typeof_arraymember)1 << ((size_t)(bit_index) - ___index___ * sizeof(typeof_arraymember) * 8))) \
            | (typeof_arraymember)(val) << ((size_t)(bit_index) - ___index___ * sizeof(typeof_arraymember) * 8); \
    } while(0)

#define BOOL_UNPACK_FROM_ARRAY(bit_index, array, array_num_members, typeof_arraymember) \
    (assert(bit_index < (array_num_members) * sizeof(typeof_arraymember) * 8), \
     (((array)[((size_t)(((size_t)bit_index) / ( sizeof(typeof_arraymember) * 8)))] \
       & ((typeof_arraymember) 1 << (((size_t)(bit_index) - ((size_t)(((size_t)bit_index) \
                                                                      / ( sizeof(typeof_arraymember) * 8))) * sizeof(typeof_arraymember) * 8)))) \
      >> ((size_t)(bit_index) - ((size_t)(((size_t)bit_index) / ( sizeof(typeof_arraymember) * 8))) * sizeof(typeof_arraymember) * 8))) \


__END_DECLS

#endif  /* HGUARD_eeac0bd03e2846dca1cd3fb01fa7efe1 */
