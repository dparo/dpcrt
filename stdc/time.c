
#include "time.h"
#include "dpcrt.h"

char *
strptime (const char *__restrict s,
          const char *__restrict f,
          struct tm *tm)
{
    assert(0);
    return 0;
}


extern time_t
timegm (struct tm *tp)
{
    assert(0);
    return 0;
}


extern size_t
strftime ( char *__restrict s,
           size_t max,
           const char *__restrict fmt,
           const struct tm *__restrict output )
{
    assert(0);
    return 0;
}


extern struct tm *
gmtime (const time_t *time)
{
    assert(0);
    return 0;
}
