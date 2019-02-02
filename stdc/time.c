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
#include "time.h"
#include "dpcrt.h"

#if 0
char *
strptime (const char *__restrict s,
          const char *__restrict f,
          struct tm *tm)
{
    (void) s, (void) f, (void) tm;
    assert(0);
    return 0;
}


extern time_t
timegm (struct tm *tp)
{
    (void) tp;
    assert(0);
    return 0;
}


extern size_t
strftime ( char *__restrict s,
           size_t max,
           const char *__restrict fmt,
           const struct tm *__restrict output )
{
    (void) s, (void) max, (void) fmt, (void) output;
    assert(0);
    return 0;
}


extern struct tm *
gmtime (const time_t *time)
{
    (void) time;
    assert(0);
    return 0;
}
#endif
