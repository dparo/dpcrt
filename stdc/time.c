
#include "time.h"
#include "utils.h"
#include "dpcrt.h"

char*
strptime (const char *__restrict source,
          const char *__restrict format,
          struct tm *output)
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
strftime (char *__restrict s,
          size_t max,
          const char *__restrict fmt,
          const struct tm *__restrict output)
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
