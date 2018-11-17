#include "types.h"


/* Use the `Ptr` typedef to avoid C strict aliasing rules and type-punning.
   The `Ptr` typedef should be defined to point to `1 byte` wide memory in order
   to make pointer arithmetic have no scale factor */
static_assert(sizeof(unsigned char) == 1, "unsigned char should be `1 byte` big in this platform.");


static_assert(sizeof(ptr_t)  == sizeof(void*), "");
static_assert(sizeof(size_t) == sizeof(void*), "");
static_assert(sizeof(ssize_t) == sizeof(size_t), "Those 2 should be equal in size but differ in signedness");
static_assert(sizeof(intptr_t) == sizeof(size_t), "Those 2 should be equal in size but differ in signedness");


static_assert(sizeof(byte_t) == 1, "");
static_assert(sizeof(I8)  == 1, "");
static_assert(sizeof(U8)  == 1, "");
static_assert(sizeof(I16) == 2, "");
static_assert(sizeof(U16) == 2, "");
static_assert(sizeof(I32) == 4, "");
static_assert(sizeof(U32) == 4, "");
static_assert(sizeof(I64) == 8, "");
static_assert(sizeof(U64) == 8, "");


// Check That types has the correct number of bytes for this architecture
static_assert(sizeof(I32_LIT(1234))  == 4, "Required for I32 max");
static_assert(sizeof(U32_LIT(1234))  == 4, "Required for U32 max");
static_assert(sizeof(I64_LIT(1234))  == 8, "Required for I64 max");
static_assert(sizeof(U64_LIT(1234))  == 8, "Required for U64 max");


