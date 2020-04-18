#ifndef HGUARD_c2eb0f431b724b318a550c630b14c94a
#define HGUARD_c2eb0f431b724b318a550c630b14c94a

#include "dpcrt_utils.h"


__BEGIN_DECLS

#include "dpcrt_utils.h"
#include "dpcrt_types.h"

static bool
streq(char *s1, char *s2)
{

    bool equal = true;

    while (equal && (*s1 != 0 || *s2 != 0))
    {
        if ((*s1++) != (*s2++))
        {
            equal = false;
            break;
        }
    }
    return equal;
}


static bool
strieq(char *s1, char *s2)
{

    bool equal = true;

    while (equal && (*s1 != 0 || *s2 != 0))
    {
        if (tolower(*s1++) != tolower(*s2++))
        {
            equal = false;
            break;
        }
    }
    return equal;
}



__END_DECLS

#endif /* HGUARD_c2eb0f431b724b318a550c630b14c94a */
