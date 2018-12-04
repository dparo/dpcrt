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

#include "dpcrt_streams.h"
#include "dpcrt_pal.h"

static void
istream__reset_buffer(IStream *istream,
                      I64 size)
{
    assert(size >= 0);
    assert((I64) ((U32) size) == size);
    istream->buffer_len = (U32) size;
    istream->buffer_it = 0;
}

static bool
istream__refill_buffer(IStream *istream)
{
    bool success = false;
    if (istream->fh == Invalid_FileHandle) {
        return (success = false);
    }
    I64 size = pal_readfile(istream->fh, istream->buffer, ISTREAM_CACHE_BUFFER_SIZE);
    if (size <= 0) {
        success = false;
    } else {
        istream__reset_buffer(istream, size);
        success = true;
    }
    return success;
}


bool
istream_init_from_file(IStream *istream,
                       char *filepath)
{
    bool success = true;
    memclr(istream, sizeof(*istream));

    const enum open_file_flags flags = FILE_RDONLY;
    istream->fh = pal_openfile(filepath, flags);
    if (istream->fh == Invalid_FileHandle) {
        success = false;
    } else {
        success = istream__refill_buffer(istream);
    }

    return success;
}


bool
istream_init_from_filehandle(IStream *istream,
                             FileHandle fh)
{
    assert(fh != Invalid_FileHandle);
    memclr(istream, sizeof(*istream));
    istream->fh = fh;
    return istream__refill_buffer(istream);
}


void
istream_deinit(IStream *istream,
               bool close_filehandle_automatically)
{
    if ( close_filehandle_automatically
         && (istream->fh != Invalid_FileHandle
             && istream->fh != Stdin_FileHandle
             && istream->fh != Stdout_FileHandle
             && istream->fh != Stderr_FileHandle)) {
        pal_closefile(istream->fh);
    }
    memclr(istream, sizeof(*istream));
}


bool
istream_peek_byte(IStream *istream, byte_t *b)
{
    bool success = false;

    if (istream->buffer_it < istream->buffer_len) {
        *b = istream->buffer[istream->buffer_it];
        success = true;
    } else {
        success = false;
    }

    return success;
}


// returns false if the advance exhausted the buffer
static inline bool
istream__advance(IStream *istream)
{
    bool exhausted = false;
    (istream->buffer_it)++;
    if (istream->buffer_it == istream->buffer_len) {
        exhausted = true;
    }
    return !exhausted;
}

static inline bool
istream__advance_read(IStream *istream)
{
    bool success = true;
    if (!istream__advance(istream)) {
        success = istream__refill_buffer(istream);
    }
    return success;
}

bool
istream_read_next_byte(IStream *istream, byte_t *byte)
{
    bool success = istream_peek_byte(istream, byte);
    if ( success == true ) {
        success = istream__advance_read(istream);
    }
    return success;
}


