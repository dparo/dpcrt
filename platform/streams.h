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
#ifndef PLATFORM__STREAMS_H
#define PLATFORM__STREAMS_H
#include "pal.h"
#include "types.h"

__BEGIN_DECLS

#define ISTREAM_CACHE_BUFFER_SIZE KILOBYTES(4)

typedef struct istream {
    filehandle_t fh;
    U32          buffer_len;
    U32          buffer_it;
    byte_t       buffer[ISTREAM_CACHE_BUFFER_SIZE];
} istream_t;


/* @TAGS:
   USAGE, EXAMPLE CODE, PATTERN:

streaming_buffer_t sb;
streaming_buffer_init_from_file("test.txt");

char b;
while(streaming_buffer_next_Byte(&sb, &b)) {
    printf("%c", b);
}*/


bool
istream_init_from_file(struct istream *istream,
                       char *filepath);

bool
istream_init_from_filehandle(struct istream *istream,
                             filehandle_t fh);

void
istream_deinit(struct istream *istream, bool close_filehandle_automatically);

bool
istream_peek_byte(struct istream *istream, byte_t *b);


static inline bool
istream_peek_char(struct istream *istream, char *c) { return istream_peek_byte(istream, (byte_t*) c); }


bool
istream_read_next_byte(struct istream *istream, byte_t *b);


static inline bool
istream_read_next_char(struct istream *istream, char *c) { return istream_read_next_byte(istream, (byte_t*) c); }


__END_DECLS


#endif  /* PLATFORM__STREAMING_BUFFER_H */
