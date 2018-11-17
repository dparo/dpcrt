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
#include "streams.h"
#include "pal.h"

static void
istream__reset_buffer(struct istream *istream,
                      I64 size)
{
    assert(size >= 0);
    assert((I64) ((U32) size) == size);
    istream->buffer_len = (U32) size;
    istream->buffer_it = 0;
}

static bool
istream__refill_buffer(struct istream *istream)
{
    bool success = false;
    if (istream->fh == invalid_filehandle) {
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
istream_init_from_file(struct istream *istream,
                       char *filepath)
{
    bool success = true;
    memclr(istream, sizeof(*istream));

    const enum open_file_flags flags = FILE_RDONLY;
    istream->fh = pal_openfile(filepath, flags);
    if (istream->fh == invalid_filehandle) {
        success = false;
    } else {
        success = istream__refill_buffer(istream);
    }

    return success;
}


bool
istream_init_from_filehandle(struct istream *istream,
                             filehandle_t fh)
{
    assert(fh != invalid_filehandle);
    memclr(istream, sizeof(*istream));
    istream->fh = fh;
    return istream__refill_buffer(istream);
}


void
istream_deinit(struct istream *istream,
               bool close_filehandle_automatically)
{
    if ( close_filehandle_automatically
         && (istream->fh != invalid_filehandle
             && istream->fh != stdin_filehandle
             && istream->fh != stdout_filehandle
             && istream->fh != stderr_filehandle)) {
        pal_closefile(istream->fh);
    }
    memclr(istream, sizeof(*istream));
}


bool
istream_peek_byte(struct istream *istream, byte_t *b)
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
istream__advance(struct  istream *istream)
{
    bool exhausted = false;
    (istream->buffer_it)++;
    if (istream->buffer_it == istream->buffer_len) {
        exhausted = true;
    }
    return !exhausted;
}

static inline bool
istream__advance_read(struct istream *istream)
{
    bool success = true;
    if (!istream__advance(istream)) {
        success = istream__refill_buffer(istream);
    }
    return success;
}

bool
istream_read_next_byte(struct istream *istream, byte_t *byte)
{
    bool success = istream_peek_byte(istream, byte);
    if ( success == true ) {
        success = istream__advance_read(istream);
    }
    return success;
}


