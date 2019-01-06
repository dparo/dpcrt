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
#ifndef HGUARD_bc5f247cae544f6f8512fd5145e13340
#define HGUARD_bc5f247cae544f6f8512fd5145e13340

#include "dpcrt_utils.h"

__BEGIN_DECLS


#if DPCRT_INCLUDE_VALGRIND_MEMCHECK

# include <valgrind/memcheck.h>

#else

/* Stub out all the valgrind defines */
#  define VALGRIND_MAKE_MEM_NOACCESS(_qzz_addr,_qzz_len)
#  define VALGRIND_MAKE_MEM_UNDEFINED(_qzz_addr,_qzz_len)
#  define VALGRIND_MAKE_MEM_DEFINED(_qzz_addr,_qzz_len)
#  define VALGRIND_MAKE_MEM_DEFINED_IF_ADDRESSABLE(_qzz_addr,_qzz_len)
#  define VALGRIND_CREATE_BLOCK(_qzz_addr,_qzz_len, _qzz_desc)
#  define VALGRIND_DISCARD(_qzz_blkindex)
#  define VALGRIND_CHECK_MEM_IS_ADDRESSABLE(_qzz_addr,_qzz_len)
#  define VALGRIND_CHECK_MEM_IS_DEFINED(_qzz_addr,_qzz_len)
#  define VALGRIND_CHECK_VALUE_IS_DEFINED(__lvalue)
#  define VALGRIND_DO_LEAK_CHECK
#  define VALGRIND_DO_ADDED_LEAK_CHECK
#  define VALGRIND_DO_CHANGED_LEAK_CHECK
#  define VALGRIND_DO_QUICK_LEAK_CHECK
#  define VALGRIND_COUNT_LEAKS(leaked, dubious, reachable, suppressed)
#  define VALGRIND_COUNT_LEAK_BLOCKS(leaked, dubious, reachable, suppressed)
#  define VALGRIND_GET_VBITS(zza,zzvbits,zznbytes)
#  define VALGRIND_SET_VBITS(zza,zzvbits,zznbytes)
#  define VALGRIND_DISABLE_ADDR_ERROR_REPORTING_IN_RANGE(_qzz_addr,_qzz_len)
#  define VALGRIND_ENABLE_ADDR_ERROR_REPORTING_IN_RANGE(_qzz_addr,_qzz_len)

#endif



__END_DECLS

#endif /* HGUARD_bc5f247cae544f6f8512fd5145e13340 */
