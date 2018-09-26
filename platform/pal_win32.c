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
#include <stdarg.h>

#include "pal.h"
#include "utils.h"

#include <windows.h>





PRINTF_STYLE(1,2)
void
fatal(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(-1);
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




void*
pal_mmap_memory( void* addr, size_t size, enum page_prot_flags prot, enum page_type_flags type )
{
    const filehandle_t fh = 0;
    return pal_mmap_aux( addr, size, prot, type, fh );
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

    if (handle == NVALID_HANDLE_VALUE)
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
            & out->last_modification.ft));

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
    LONG l = CompareFileTime(
        & (ft1->ft),
        & (ft2->ft)
        );
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
    LoadLibrary(path);
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
