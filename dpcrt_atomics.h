/*
 * Copyright (C) 2019  Davide Paro
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
#ifndef HGUARD_320d615d2d294620a7e79f97c85f49dc
#define HGUARD_320d615d2d294620a7e79f97c85f49dc

#include "dpcrt_utils.h"


__BEGIN_DECLS



#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9))


#  define /* type */ atomic_load(/* type* */ ptr) \
    __atomic_load_n(ptr, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_store(/* type* */ ptr, /* type */ value)  \
    __atomic_store_n(ptr, value, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_exchange(/* type* */ ptr, /* type */ value) \
    __atomic_compare_exchange_n(ptr, value, __ATOMIC_SEQ_CST)
#  define /* bool */ atomic_compare_exchange(/* [TYPE *] */ ptr, /* [TYPE *] */ expected_out_addr, /* [TYPE] */ desired) \
    __atomic_compare_exchange_n(ptr, expected_out_addr, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)





#  define /* type */ atomic_add_fetch(/* type* */ ptr, /* type */ value) \
    __atomic_add_fetch(ptr, value, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_sub_fetch(/* type* */ ptr, /* type */ value) \
    __atomic_sub_fetch(ptr, value, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_and_fetch(/* type* */ ptr, /* type */ value) \
    __atomic_and_fetch(ptr, value, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_xor_fetch(/* type* */ ptr, /* type */ value) \
    __atomic_xor_fetch(ptr, value, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_or_fetch(/* type* */ ptr, /* type */ value) \
    __atomic_or_fetch(ptr, value, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_nand_fetch(/* type* */ ptr, /* type */ value) \
    __atomic_nand_fetch(ptr, value, __ATOMIC_SEQ_CST)

#  define /* type */ atomic_fetch_add(/* type* */ ptr, /* type */ value) \
    __atomic_fetch_add(ptr, value, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_fetch_sub(/* type* */ ptr, /* type */ value) \
    __atomic_fetch_sub(ptr, value, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_fetch_and(/* type* */ ptr, /* type */ value) \
    __atomic_fetch_and(ptr, value, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_fetch_xor(/* type* */ ptr, /* type */ value) \
    __atomic_fetch_xor(ptr, value, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_fetch_or(/* type* */ ptr, /* type */ value) \
    __atomic_fetch_or(ptr, value, __ATOMIC_SEQ_CST)
#  define /* type */ atomic_fetch_nand(/* type* */ ptr, /* type */ value) \
    __atomic_fetch_nand(ptr, value, __ATOMIC_SEQ_CST)



    

    
/* Only for `bool` or `char types */
#  define /* void */ atomic_test_and_set(/* char* || bool* */ ptr)    \
    __atomic_test_and_set(ptr, __ATOMIC_SEQ_CST)
#  define /* void */ atomic_clear(/* char* || bool* */ ptr) \
    __atomic_clear(ptr, __ATOMIC_SEQ_CST)
#  define /* void */ atomic_thread_fence()      \
    __atomic_thread_fence(__ATOMIC_SEQ_CST)
#  define /* void */ atomic_signal_fence()      \
    __atomic_signal_fence(__ATOMIC_SEQ_CST)
#  define /* bool */ atomic_always_lock_free(/* size_t */ size, /* void* */ ptr) \
    __atomic_always_lock_free(size, ptr)
#  define /* bool */ atomic_is_lock_free(/* size_t */ size, /* void* */ ptr) \
    __atomic_is_lock_free(size, ptr)
    
#else
#  error "Needs support for this platform"
#endif




    __END_DECLS

#endif /* HGUARD_320d615d2d294620a7e79f97c85f49dc */
