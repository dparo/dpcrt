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
#ifndef HGUARD_abf720910adf4d2f9e7b4343fbf5cf7c
#define HGUARD_abf720910adf4d2f9e7b4343fbf5cf7c

#include "dpcrt_utils.h"

__BEGIN_DECLS

#ifndef	_ERRNO_H
#define	_ERRNO_H 1


/* Some defines for common values of errno feel free to add more
   as the need arises */
#if __PAL_WINDOWS__
/* If they are not equal remember to do preprocessor if for conditional compilation definition */
# error "REMAINDER :: Check if the ERRNO LINUX constants provided below are the same value even under WINDOWS OS"
#endif

#define EAGAIN 11



int *__errno_location (void) ATTRIB_NOTHROW ATTRIB_CONST;

# define errno (*__errno_location ())


#endif  /* _ERRNO_H */


__END_DECLS

#endif /* HGUARD_abf720910adf4d2f9e7b4343fbf5cf7c */
