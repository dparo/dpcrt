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
#ifndef HGUARD_b63a9f78798c4a55901e24665e7157b3
#define HGUARD_b63a9f78798c4a55901e24665e7157b3

#include "dpcrt_utils.h"


__BEGIN_DECLS


DLL_LOCAL ATTRIB_WEAK
void on_dll_load(void)    
{
    /* Override this function is another source file to be
       notified when the DLL gets successfully loaded
       (equivalent to `main` function of a "normal" executable) */
}

DLL_LOCAL ATTRIB_WEAK
void on_dll_unload(void)
{
    /* Override this function is another source file to be
       notified when the DLL gets successfully unloaded */
}






/* Link with the OS specific way to load the DLL */

#if __DPCRT_WINDOWS
# include <windows.h>
BOOL WINAPI
DllMain(_In_ HINSTANCE hinstDLL,
        _In_ DWORD     fdwReason,
        _In_ LPVOID    lpvReserved)
{
    
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        on_dll_load();
    }
    else if(fdwReason == DLL_PROCESS_DETACH)
    {
        on_dll_unload();
    }
    return true;
}

#else

# if __GNUC__

__attribute__((constructor))
void __gnu_on_dll_load(void)
{
    on_dll_load();
}
__attribute__((destructor))
void __gnu_on_dll_unload(void)
{
    on_dll_unload();
}

# else
#  error "Compiler or platform not supported or not yet added"
# endif
#endif



__END_DECLS

#endif /* HGUARD_b63a9f78798c4a55901e24665e7157b3 */
