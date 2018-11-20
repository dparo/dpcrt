#ifndef HGUARD_e138864469634ee39cde1436e8890ed3
#define HGUARD_e138864469634ee39cde1436e8890ed3

#include "utils.h"


__BEGIN_DECLS

extern void *
malloc (size_t size)
    ATTRIB_NOTHROW ATTRIB_MALLOC ATTRIB_NODISCARD;

extern void *
calloc (size_t nmemb, size_t size)
    ATTRIB_NOTHROW ATTRIB_MALLOC ATTRIB_NODISCARD;

extern void *
valloc ( size_t size )
    ATTRIB_NOTHROW ATTRIB_MALLOC ATTRIB_NODISCARD;

extern void *
realloc (void *ptr, size_t size)
    ATTRIB_NOTHROW ATTRIB_NODISCARD;

extern void
free ( void *ptr )
    ATTRIB_NOTHROW;


__END_DECLS

#endif  /* HGUARD_e138864469634ee39cde1436e8890ed3 */
