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
#ifndef HGUARD_20c269e3891149538640102998c1f283
#define HGUARD_20c269e3891149538640102998c1f283

#include "dpcrt_utils.h"
#include "dpcrt_types.h"

__BEGIN_DECLS



struct tm
{
    int tm_sec;                 /* Seconds.	[0-60] (1 leap second) */
    int tm_min;                 /* Minutes.	[0-59] */
    int tm_hour;                /* Hours.	[0-23] */
    int tm_mday;                /* Day.		[1-31] */
    int tm_mon;                 /* Month.	[0-11] */
    int tm_year;                /* Year	- 1900.  */
    int tm_wday;                /* Day of week.	[0-6] */
    int tm_yday;                /* Days in year.[0-365]	*/
    int tm_isdst;               /* DST.		[-1/0/1]*/

#if	 __GNUC__
    /* GNU C Extension only */
    long int    tm_gmtoff;	  /* Seconds east of UTC.  */
    const char *tm_zone;        /* Timezone abbreviation.  */
# else
    // long int    ___padding1___;  /* Seconds east of UTC.  */
    // const char *___padding2___;  /* Timezone abbreviation.  */
# endif
};


extern char*
strptime (const char *restrict source,
          const char *restrict format,
          struct tm *output)
    ATTRIB_NOTHROW;

extern size_t
strftime (char *restrict s,
          size_t max,
          const char *restrict fmt,
          const struct tm *restrict output)
    ATTRIB_NOTHROW;

extern time_t
timegm (struct tm *tp)
    ATTRIB_NOTHROW;

extern struct tm *
gmtime (const time_t *time)
    ATTRIB_NOTHROW;



__END_DECLS

#endif /* HGUARD_20c269e3891149538640102998c1f283 */
