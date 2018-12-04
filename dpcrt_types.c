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

#include "dpcrt_types.h"



/* Use the `Ptr` typedef to avoid C strict aliasing rules and type-punning.
   The `Ptr` typedef should be defined to point to `1 byte` wide memory in order
   to make pointer arithmetic have no scale factor */
static_assert(sizeof(unsigned char) == 1, "unsigned char should be `1 byte` in this platform.");


static_assert(sizeof(size_t)   == sizeof(void*), "");
static_assert(sizeof(ssize_t)  == sizeof(size_t), "Those 2 should be equal in size but differ in signedness");
static_assert(sizeof(intptr_t) == sizeof(size_t), "Those 2 should be equal in size but differ in signedness");


static_assert(sizeof(byte_t) == 1, "");
static_assert(sizeof(I8)     == 1, "");
static_assert(sizeof(U8)     == 1, "");
static_assert(sizeof(I16)    == 2, "");
static_assert(sizeof(U16)    == 2, "");
static_assert(sizeof(I32)    == 4, "");
static_assert(sizeof(U32)    == 4, "");
static_assert(sizeof(I64)    == 8, "");
static_assert(sizeof(U64)    == 8, "");


// Check That types has the correct number of bytes for this architecture
static_assert(sizeof( I8_LIT(123456)) == 1, "");
static_assert(sizeof(I16_LIT(123456)) == 2, "");
static_assert(sizeof(I32_LIT(123456)) == 4, "");
static_assert(sizeof(I64_LIT(123456)) == 8, "");

static_assert(sizeof( U8_LIT(123456)) == 1, "");
static_assert(sizeof(U16_LIT(123456)) == 2, "");
static_assert(sizeof(U32_LIT(123456)) == 4, "");
static_assert(sizeof(U64_LIT(123456)) == 8, "");


static_assert(sizeof( U8_MAX) == 1, "");
static_assert(sizeof(U16_MAX) == 2, "");
static_assert(sizeof(U32_MAX) == 4, "");
static_assert(sizeof(U64_MAX) == 8, "");

static_assert(sizeof( I8_MIN) == 1, "");
static_assert(sizeof( I8_MAX) == 1, "");
static_assert(sizeof(I16_MIN) == 2, "");
static_assert(sizeof(I16_MAX) == 2, "");
static_assert(sizeof(I32_MIN) == 4, "");
static_assert(sizeof(I32_MAX) == 4, "");
static_assert(sizeof(I64_MIN) == 8, "");
static_assert(sizeof(I64_MAX) == 8, "");






static_assert(offsetof(Str32Hdr, bufsize) == offsetof(Str32, bufsize), "");
static_assert(offsetof(Str32Hdr, len)     == offsetof(Str32, len), "");
static_assert(offsetof(Str32Hdr, bufsize) == offsetof(PStr32, bufsize), "");
static_assert(offsetof(Str32Hdr, len)     == offsetof(PStr32, len), "");


