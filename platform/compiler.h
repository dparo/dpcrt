/*
 * Copyright (C) 2018  Davide Paro
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
#ifndef HGUARD_39b345774ad64521a296089279f0123a
#define HGUARD_39b345774ad64521a296089279f0123a


/* @NOTE :: IMPORTANT
   =======================================

   This file should not depend on any other file (eg it should not
   include anything else, other than the features already provided
   from the compiler the language, and the build system.
 */

#ifndef __PAL_WINDOWS__
#  define __PAL_WINDOWS__ _WIN16 || _WIN32 || _WIN64
#endif

# if _WIN16
#   define __PAL_WINDOWS_VERSION__ (16)
# endif
# if _WIN32
#   define __PAL_WINDOWS_VERSION__ (32)
# endif
# if _WIN64
#   define __PAL_WINDOWS_VERSION__ (64)
# endif


#ifndef __PAL_LINUX__
#  if __linux__
#    define __PAL_LINUX__ (1)
#  endif
#endif

#ifndef __PAL_APPLE__
#  if __APPLE__
#    define __PAL_APPLE__ (1)
#  endif
#endif


#if	__cplusplus
#  ifndef __BEGIN_DECLS
#    define __BEGIN_DECLS extern "C" {
#  endif
#  ifndef __END_DECLS
#    define __END_DECLS }
#  endif
#else
#  ifndef __BEGIN_DECLS
#    define __BEGIN_DECLS
#  endif
#  ifndef __END_DECLS
#    define __END_DECLS
#  endif
#endif


__BEGIN_DECLS


/* Standard versions defined by the compiler. Example usage
   # if __STDC_VERSION__ >= C11_STD_VERSION
   {| Do things here that requires at least c11 support |}
   # endif
*/
#define STD_C89_VERSION   199409L
#define STD_ANSIC_VERSION C89_STD
#define STD_C99_VERSION   199901L
#define STD_C11_VERSION   201112L
#define STD_C17_VERSION   201710L

#if __STDC_VERSION__ < STD_C99_VERSION
#   define restrict
#   define __restrict
#else
#   define __restrict restrict
#endif

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)


#if defined  __GNUC__ || defined __GNUG__ || defined __clang__
# define ARRAY_LEN(arr)                                                 \
    (sizeof(arr) / sizeof((arr)[0])                                     \
     + sizeof(typeof(int[1 - 2 *                                        \
                         !!__builtin_types_compatible_p(typeof(arr), typeof(&arr[0]))])) * 0)
#else
# define ARRAY_LEN(A)                           \
    (sizeof(A) / sizeof((A)[0]))
#endif

/* Compute the length of a c-string literal known at compile time */
#define STRLIT_LEN(S) (ARRAY_LEN(S) - 1)
    



// Supports for functions that gets run before entering `main`
// example CONSTRUCT(my_init)
// void my_init(void) { ... }
#if __GNUC__
#  define ATTRIB_CONSTRUCT(_func) static void _func (void) __attribute__((constructor));
#  define ATTRIB_DESTRUCT(_func) static void _func (void) __attribute__((destructor));
#  define ATTRIB_DEPRECATED __attribute__((deprecated))
#  define ATTRIB_PURE __attribute__((pure))
#  define ATTRIB_CONST __attribute__((const))
#  define ATTRIB_FUNCTIONAL ATTRIB_CONST
#  define ATTRIB_NONNULL(...)  __attribute__((nonnull(__VA_ARGS__)))
#  define ATTRIB_MALLOC __attribute__((malloc))
#  define ATTRIB_NODISCARD __attribute__((warn_unused_result)) /* The return value from the function should be checked */
#  define ATTRIB_LEAF      __attribute__((leaf))
#  define ATTRIB_NOTHROW   __attribute__((nothrow))
#  define ATTRIB_PRINTF(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format(printf, (STRING_INDEX), (FIRST_TO_CHECK))))


#elif defined (_MSC_VER) && (_MSC_VER >= 1500)
#  define ATTRIB_CONSTRUCT(_func)                                       \
    static void _func(void);                                            \
    static int _func ## _wrapper(void) { _func(); return 0; }           \
    __pragma(section(".CRT$XCU",read))                                  \
        __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _wrapper;

#  define ATTRIB_DESTRUCT(_func)                                        \
    static void _func(void);                                            \
    static int _func ## _constructor(void) { atexit (_func); return 0; } \
    __pragma(section(".CRT$XCU",read))                                  \
        __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _constructor;


/* @TODO :: Some of these defines expands to `GCC` builtins. Those are probably
   not going to work under `MSVC`. Check if `MSVC` provides something similar
   for those attribs and make the macro expand to the correct text.
   If the equivalent attribute under `MSVC` just make the macro expand to `EMPTY` */
#  define ATTRIB_NONNULL(...)  __attribute__((nonnull(__VA_ARGS__)))
#  define ATTRIB_MALLOC __attribute__((malloc))
#  define ATTRIB_NODISCARD _Check_return_
#  define ATTRIB_LEAF      __attribute__((leaf))
#  define ATTRIB_NOTHROW   __declspec(nothrow)
#  define ATTRIB_PRINTF(STRING_INDEX, FIRST_TO_CHECK) __attribute__ ((format(printf, (STRING_INDEX), (FIRST_TO_CHECK))))
#  define ATTRIB_PURE
#  define ATTRIB_CONST
#  define ATTRIB_FUNCTIONAL ATTRIB_CONST
#else
# error "Not supported platform, or need to add it yourself"
#endif
// #################################################


// Compiler DLL Support, please refer to page        https://gcc.gnu.org/wiki/Visibility
#if __PAL_WINDOWS__ || __CYGWIN__
  #if BUILDING_DLL
    #if __GNUC__
      #define DLL_GLOBAL __attribute__ ((dllexport))
    #else
      #define DLL_GLOBAL __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #if __GNUC__
      #define DLL_GLOBAL __attribute__ ((dllimport))
    #else
      #define DLL_GLOBAL __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DLL_GLOBAL __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define DLL_GLOBAL
    #define DLL_LOCAL
  #endif
#endif




# ifndef offsetof
#   if __GNUC__
#     define offsetof(type, member) __builtin_offsetof (type, member)
#   else
#     define offsetof(type, member) ((size_t)&(((type *)0)->member))
#   endif
# endif




#define STRINGIFY(x) #x
#define __AT_SRC__ __FILE__ ":" STRINGIFY(__LINE__)
#define CONCAT_(a, ...) a ## __VA_ARGS__
#define CONCAT(a, ...) CONCAT_(a, __VA_ARGS__)
/* Example of deferring */
/* #define A() 123 */
/* A() // Expands to 123 */
/* DEFER(A)() // Expands to A () because it requires one more scan to fully expand */
/* EXPAND(DEFER(A)()) // Expands to 123, because the EXPAND macro forces another scan */

#define EMPTY()
#define DEFER(id) id EMPTY()
#define OBSTRUCT(...) __VA_ARGS__ DEFER(EMPTY)()
#define EXPAND(...) __VA_ARGS__




/* ===================================
   Static Assertion 
   =================================== */


#ifndef DPCC_STATIC_ASSERT
#  if __STDC_VERSION__ >= STD_C11_VERSION
#    define DPCC_STATIC_ASSERT(cond, msg) _Static_assert((cond), msg)
#  else
#    define DPCC_STATIC_ASSERT(cond, msg) struct CONCAT(__dpccasrt__, __LINE__) { int a[!cond ? -1 : 0]; }
#  endif
#endif

#define static_assert DPCC_STATIC_ASSERT


__END_DECLS

#endif  /* HGUARD_39b345774ad64521a296089279f0123a */
