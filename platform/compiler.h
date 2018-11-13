/*
 * Copyright (C) 2018  Davide Paro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef COMPILER_H
#define COMPILER_H


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



// Supports for functions that gets run before entering `main`
// example CONSTRUCT(my_init)
// void my_init(void) { ... }
#if __GNUC__
#  define CONSTRUCT(_func) static void _func (void) __attribute__((constructor));
#  define DESTRUCT(_func) static void _func (void) __attribute__((destructor));
#  define DEPRECATED __attribute__((deprecated))
#  define ATTRIB_PURE __attribute__((pure))
#  define ATTRIB_CONST __attribute__((const))
#  define ATTRIB_FUNCTIONAL ATTRIB_CONST
#elif defined (_MSC_VER) && (_MSC_VER >= 1500)
#  define CONSTRUCT(_func)                                              \
    static void _func(void);                                            \
    static int _func ## _wrapper(void) { _func(); return 0; }           \
    __pragma(section(".CRT$XCU",read))                                  \
        __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _wrapper;

#  define DESTRUCT(_func)                                               \
    static void _func(void);                                            \
    static int _func ## _constructor(void) { atexit (_func); return 0; } \
    __pragma(section(".CRT$XCU",read))                                  \
        __declspec(allocate(".CRT$XCU")) static int (* _array ## _func)(void) = _func ## _constructor;
#  define ATTRIB_PURE
#  define ATTRIB_CONST
#  define ATTRIB_FUNCTIONAL ATTRIB_CONST
#else
#  error "You will need constructor support for your compiler"
#endif
// #################################################


#if __GNUC__
#   define PRINTF_STYLE(STRING_INDEX, FIRST_TO_CHECK)                   \
    __attribute__ ((format(printf, (STRING_INDEX), (FIRST_TO_CHECK))))

#   define NON_NULL_PARAM(NUM) __attribute__((nonnull (NUM)));

#elif defined (_MSC_VER) && (_MSC_VER >= 1500)
#   define PRINTF_STYLE(STRING_INDEX, FIRST_TO_CHECK)
#else
#error "You will need constructor support for your compiler"
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






__END_DECLS

#endif
