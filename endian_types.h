#ifndef HGUARD_c5de58dedcab4b82a1d15b2f2f956495
#define HGUARD_c5de58dedcab4b82a1d15b2f2f956495

#include "types.h"


__BEGIN_DECLS

    
/* Platform Indipendent Little Endian types.
   Use `unions` to enforce strong typing for byte order dependent types. */
typedef union { I8 val; } I8LE;
typedef union { U8 val; } U8LE;
typedef union { I16 val; } I16LE;
typedef union { U16 val; } U16LE;
typedef union { I32 val; } I32LE;
typedef union { U32 val; } U32LE;
typedef union { I64 val; } I64LE;
typedef union { U64 val; } U64LE;
typedef union { U32 u32val; F32 f32val; } F32LE;
typedef union { U64 u64val; F64 f64val; } F64LE;

/* Platform Indipendent Big Endian types.
   Use `unions` to enforce strong typing for byte order dependent types. */
typedef union { I8 val; } I8BE;
typedef union { U8 val; } U8BE;
typedef union { I16 val; } I16BE;
typedef union { U16 val; } U16BE;
typedef union { I32 val; } I32BE;
typedef union { U32 val; } U32BE;
typedef union { I64 val; } I64BE;
typedef union { U64 val; } U64BE;
typedef union { U32 u32val; F32 f32val; } F32BE;
typedef union { U64 u64val; F64 f64val; } F64BE;

#if !__GNUC__
ATTRIB_CONST static inline U16 bswap_u16(const U16 u16) {
    return ( (U16)   ((u16 & U16_LIT(0xff00)) >> 8)
             | (U16) ((u16 & U16_LIT(0x00ff)) << 8)
        );
}

ATTRIB_CONST static inline U32 bswap_u32(const U32 u32) {
    return ( (U32)   ((u32 & U32_LIT(0xff000000)) >> 24)
             | (U32) ((u32 & U32_LIT(0x00ff0000)) >> 8)
             | (U32) ((u32 & U32_LIT(0x0000ff00)) << 8)
             | (U32) ((u32 & U32_LIT(0x000000ff)) << 24)
        );
}

ATTRIB_CONST static inline U64 bswap_u64(const U64 u64) {
    return ( (U64)   ((u64 & U64_LIT(0xff00000000000000)) >> 56)
             | (U64) ((u64 & U64_LIT(0x00ff000000000000)) >> 40)
             | (U64) ((u64 & U64_LIT(0x0000ff0000000000)) >> 24)
             | (U64) ((u64 & U64_LIT(0x000000ff00000000)) >> 8)
             | (U64) ((u64 & U64_LIT(0x00000000ff000000)) << 8)
             | (U64) ((u64 & U64_LIT(0x0000000000ff0000)) << 24)
             | (U64) ((u64 & U64_LIT(0x000000000000ff00)) << 40)
             | (U64) ((u64 & U64_LIT(0x00000000000000ff)) << 56)
        );
}

#endif

#if __GNUC__
/* If using GCC override the functions definitions to use the already provided builtins instead */
#  define bswap_u16(u16) __builtin_bswap16(u16)
#  define bswap_u32(u32) __builtin_bswap32(u32)
#  define bswap_u64(u64) __builtin_bswap64(u64)
#endif

# warning "THIS THINGS SHOULD BE WRAPPED INSIDE PLATFORM ENDIANS CHECKS"
ATTRIB_CONST static inline I8  i8le_to_i8(I8LE i8le)     { return i8le.val; }
ATTRIB_CONST static inline U8  u8le_to_u8(U8LE u8le)     { return u8le.val; }
ATTRIB_CONST static inline I16 i16le_to_i16(I16LE i16le) { return i16le.val; }
ATTRIB_CONST static inline U16 u16le_to_u16(U16LE u16le) { return u16le.val; }
ATTRIB_CONST static inline I32 i32le_to_i32(I32LE i32le) { return i32le.val; }
ATTRIB_CONST static inline U32 u32le_to_u32(U32LE u32le) { return u32le.val; }
ATTRIB_CONST static inline I64 i64le_to_i64(I64LE i64le) { return i64le.val; }
ATTRIB_CONST static inline U64 u64le_to_u64(U64LE u64le) { return u64le.val; }
ATTRIB_CONST static inline F32 f32le_to_f32(F32LE f32le) { return f32le.f32val; }
ATTRIB_CONST static inline F64 f64le_to_f64(F64LE f64le) { return f64le.f64val; }




# warning "THIS THINGS SHOULD BE WRAPPED INSIDE PLATFORM ENDIANS CHECKS"
ATTRIB_CONST static inline I8  i8be_to_i8(I8BE i8be)     { return i8be.val; }
ATTRIB_CONST static inline U8  u8be_to_u8(U8BE u8be)     { return u8be.val; }
ATTRIB_CONST static inline I16 i16be_to_i16(I16BE i16be) { return i16be.val; }
ATTRIB_CONST static inline U16 u16be_to_u16(U16BE u16be) { return u16be.val; }
ATTRIB_CONST static inline I32 i32be_to_i32(I32BE i32be) { return i32be.val; }
ATTRIB_CONST static inline U32 u32be_to_u32(U32BE u32be) { return u32be.val; }
ATTRIB_CONST static inline I64 i64be_to_i64(I64BE i64be) { return i64be.val; }
ATTRIB_CONST static inline U64 u64be_to_u64(U64BE u64be) { return u64be.val; }
ATTRIB_CONST static inline F32 f32be_to_f32(F32BE f32be) { return f32be.f32val; }
ATTRIB_CONST static inline F64 f64be_to_f64(F64BE f64be) { return f64be.f64val; }


__END_DECLS

#endif /* HGUARD_c5de58dedcab4b82a1d15b2f2f956495 */
