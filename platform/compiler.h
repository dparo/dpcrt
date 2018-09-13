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



#define __WINDOWS__ _WIN16 || _WIN32 || _WIN64

#ifdef	__cplusplus
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


/* Compiler dependent defines AND OR wrappers */

// Supports for functions that gets run before entering in main
// example CONSTRUCT(my_init)
// void my_init(void) { ... }
#ifdef __GNUC__
#  define CONSTRUCT(_func) static void _func (void) __attribute__((constructor));
#  define DESTRUCT(_func) static void _func (void) __attribute__((destructor));
#  define DEPRECATED __attribute__((deprecated))
#  define PURE __attribute__((pure))
#  define CONST __attribute__((const))
#  define FUNCTIONAL CONST
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
#  define PURE
#  define CONST
#  define FUNCTIONAL CONST
#else
#  error "You will need constructor support for your compiler"
#endif
// #################################################


#ifdef __GNUC__
#   define PRINTF_STYLE(STRING_INDEX, FIRST_TO_CHECK)                   \
    __attribute__ ((format(printf, (STRING_INDEX), (FIRST_TO_CHECK))))

#   define NON_NULL_PARAM(NUM) __attribute__((nonnull (NUM)));

#elif defined (_MSC_VER) && (_MSC_VER >= 1500)
#   error "Implement me for windows"
#else
#error "You will need constructor support for your compiler"
#endif
// #################################################



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



// Compiler DLL Support, please refer to page        https://gcc.gnu.org/wiki/Visibility
#if __WINDOWS__ || __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DLL_GLOBAL __attribute__ ((dllexport))
    #else
      #define DLL_GLOBAL __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
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
