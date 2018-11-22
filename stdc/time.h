#ifndef HGUARD_20c269e3891149538640102998c1f283
#define HGUARD_20c269e3891149538640102998c1f283

#include "utils.h"


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
    long int    ___padding1___;  /* Seconds east of UTC.  */
    const char *___padding2___;  /* Timezone abbreviation.  */
# endif
};


extern char*
strptime (const char *__restrict source,
          const char *__restrict format,
          struct tm *output)
    ATTRIB_NOTHROW;

extern size_t
strftime (char *__restrict s,
          size_t max,
          const char *__restrict fmt,
          const struct tm *__restrict output)
    ATTRIB_NOTHROW;

extern time_t
timegm (struct tm *tp)
    ATTRIB_NOTHROW;

extern struct tm *
gmtime (const time_t *time)
    ATTRIB_NOTHROW;



__END_DECLS

#endif /* HGUARD_20c269e3891149538640102998c1f283 */
