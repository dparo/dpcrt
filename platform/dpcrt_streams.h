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

#ifndef HGUARD_c0ebfee833bb44829631b83fbe8d4482
#define HGUARD_c0ebfee833bb44829631b83fbe8d4482


#include "dpcrt_pal.h"
#include "dpcrt_types.h"

__BEGIN_DECLS

#define ISTREAM_CACHE_BUFFER_SIZE KILOBYTES(4)

typedef struct IStream {
    FileHandle   fh;
    U32          buffer_len;
    U32          buffer_it;
    byte_t       buffer[ISTREAM_CACHE_BUFFER_SIZE];
} IStream;


/* @TAGS:
   USAGE, EXAMPLE CODE, PATTERN:

streaming_buffer_t sb;
streaming_buffer_init_from_file("test.txt");

char b;
while(streaming_buffer_next_Byte(&sb, &b)) {
    printf("%c", b);
}*/


bool
istream_init_from_file(IStream *istream,
                       char *filepath);

bool
istream_init_from_filehandle(IStream *istream,
                             FileHandle fh);

void
istream_deinit(IStream *istream, bool close_filehandle_automatically);

bool
istream_peek_byte(IStream *istream, byte_t *b);


static inline bool
istream_peek_char(IStream *istream, char *c) { return istream_peek_byte(istream, (byte_t*) c); }


bool
istream_read_next_byte(IStream *istream, byte_t *b);


static inline bool
istream_read_next_char(IStream *istream, char *c) { return istream_read_next_byte(istream, (byte_t*) c); }


__END_DECLS


#endif  /* HGUARD_c0ebfee833bb44829631b83fbe8d4482 */
