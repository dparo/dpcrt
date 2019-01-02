/*
 * Copyright (C) 2019  Davide Paro
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
#ifndef HGUARD_9450a2815fb5433089759d98d74dbcad
#define HGUARD_9450a2815fb5433089759d98d74dbcad

__BEGIN_DECLS

#include "dpcrt_utils.h"

#ifndef __FILE_defined
#define ____FILE_defined 1
#define __FILE_defined 1
struct _IO_FILE;
typedef struct _IO_FILE FILE;
#endif

/* Standard streams.  */
extern struct _IO_FILE *stdin;		/* Standard input stream.  */
extern struct _IO_FILE *stdout;		/* Standard output stream.  */
extern struct _IO_FILE *stderr;		/* Standard error output stream.  */
/* C89/C99 say they're macros.  Make them happy.  */
#define stdin stdin
#define stdout stdout
#define stderr stderr


#include <stdc/stdarg.h>

#ifndef _STDIO_H
#define _STDIO_H	1

int fprintf (FILE *restrict f, const char *restrict fmt, ...);
int printf  (const char *restrict fmt, ...);
int sprintf (char *restrict s, const char *restrict fmt, ...) ATTRIB_NOTHROW;


extern int vfprintf  (FILE * restrict __s, const char *restrict fmt,  va_list ap);
extern int vprintf   (const char *restrict format, va_list ap);
extern int vsprintf  (char *restrict s, const char *restrict fmt, va_list arg) ATTRIB_NOTHROW;
extern int snprintf  (char *restrict s, size_t len, const char *restrict fmt, ...) ATTRIB_NOTHROW ATTRIB_PRINTF(3, 4);
extern int vsnprintf (char *restrict s, size_t len, const char *restrict fmt, va_list ap) ATTRIB_NOTHROW ATTRIB_PRINTF(3, 0);


extern int fputs (const char *restrict s, FILE *restrict f);
extern int fgetc (FILE *f);
extern int puts  (const char *s);


FILE*    fopen  (const char *restrict path, const char *restrict flags) ATTRIB_NODISCARD;
int      fclose (FILE *f);
int      fflush (FILE *f);
size_t   fread  (void *restrict buf, size_t sizeof_1_element, size_t count_of_elements, FILE *restrict f) ATTRIB_NODISCARD;
size_t   fwrite (const void *restrict buf, size_t sizeof_1_element, size_t count_of_elements, FILE *restrict f);
int      fseek  (FILE *f, long int offset, int whence);
long int ftell  (FILE *f) ATTRIB_NODISCARD;



void perror (const char *s);


#endif

__END_DECLS

#endif  /* HGUARD_9450a2815fb5433089759d98d74dbcad */
