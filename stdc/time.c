
#include "time.h"
#include "dpcrt.h"

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
