#ifndef MALLOC_H
#define MALLOC_H

#include "utils.h"


__BEGIN_DECLS

extern void *
malloc (size_t size)
    ATTRIB_NOTHROW ATTRIB_MALLOC ATTRIB_NODISCARD;

/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
extern void *
calloc (size_t nmemb, size_t size)
    ATTRIB_NOTHROW ATTRIB_MALLOC ATTRIB_NODISCARD;


extern void *
valloc ( size_t size )
    ATTRIB_NOTHROW ATTRIB_MALLOC ATTRIB_NODISCARD;

extern void *
realloc (void *ptr, size_t size)
    ATTRIB_NOTHROW ATTRIB_NODISCARD;

extern void *
free ( void *ptr )
    ATTRIB_NOTHROW;


__END_DECLS

#endif
