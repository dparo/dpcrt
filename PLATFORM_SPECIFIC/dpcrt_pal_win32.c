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

#include <stdarg.h>

#include "dpcrt_pal.h"
#include "dpcrt_utils.h"

#include <windows.h>



void
pal_exit(int status)
{
    ExitProcess(status);
}

void
pal_abort(void)
{
    abort();
}


ATTRIB_PRINTF(1,2)
void
pal_fatal(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    pal_exit(-1);
}


static void
win32_print_stack_trace(void)
{
    invalid_code_path("WIN32 Print stack trace needs implementation");
}


void
pal_print_stack_trace(void)
{
    win32_print_stack_trace();
}

static void sigsegv_linux_callback(int sig)
{
    fprintf(stderr, "Error: signal %d: %s\n", sig, strsignal(sig));
    pal_print_stack_trace();
    exit(-1);
}



bool
pal_stat_filetime(char *filepath, struct stat_filetime *out)
{
    HANDLE handle = CreateFileA(
        filepath,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL );

    if (handle == INVALID_HANDLE_VALUE)
    {
        success = false;
        memclr(out, sizeof(*out));
    }
    else
    {
        BOOL b = GetFileTime(
            handle,
            NULL
            & out->last_access.ft,
            & out->last_modification.ft);

        if (b == 0)
        {
            memclr(out, sizeof(*out));
            success = false;
        }

        b = CloseHandle(handle);
        assert(b != 0);
    }
    return success;
}


int
pal_filetime_cmp(struct filetime *ft1,
                 struct filetime *ft2)
{
    LONG l = CompareFileTime(& ft1->ft,
                             & ft2->ft );
    return (int) l;
}







dll_handle_t
pal_dll_open(char *path)
{
    dll_handle_t result = 0;
    result = LoadLibrary(path);
    return result;
}

void
pal_dll_close(dll_handle_t handle)
{
    assert(handle != invalid_dll_handle);
    FreeLibrary(handle);
}

void *
pal_get_proc_addr(dll_handle_t handle,
                  const char *symbol_name)
{
    void *result = NULL;
    assert(handle != invalid_dll_handle);
    result = (void*) GetProcAddress(handle, symbol_name);
    return result;
}
