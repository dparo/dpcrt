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

#pragma once

#include "dpcrt_utils.h"
#include <stdc/stdio.h>
#include <stdc/errno.h>
#include <stdc/string.h>



#define ERR(...)                                         \
    do {                                                 \
        fprintf(stderr, __VA_ARGS__);                    \
        fflush(stderr);                                  \
        assert(0);                                       \
        exit(-1);                                        \
    } while (0)


enum page_prot_flags {
    PAGE_PROT_NONE  = 0,         /* Memory cannot be accessed */
    PAGE_PROT_READ  = (1 << 0),  /* Memory can be read */
    PAGE_PROT_WRITE = (1 << 1),  /* Memory can be written  */
    PAGE_PROT_EXEC  = (1 << 2),  /* Memory can be executed */
};


enum page_type_flags {
    PAGE_FIXED     = 0,
    PAGE_SHARED    = (1 << 0),
    PAGE_PRIVATE   = (1 << 1),
    PAGE_ANONYMOUS = (1 << 2),
    PAGE_LOCKED    = (1 << 3),
    PAGE_GROWSDOWN = (1 << 4),
    PAGE_POPULATE  = (1 << 5),
    PAGE_HUGETLB   = (1 << 6),
    PAGE_HUGE_2MB  = (1 << 7),
    PAGE_HUGE_1GB  = (1 << 8),
};

enum page_remap_flags {
    PAGE_REMAP_NONE = 0,
    PAGE_REMAP_MAYMOVE = (1 << 0),
    PAGE_REMAP_FIXED = (1 << 1),
};


enum open_file_flags {
    FILE_NONE      = 0,
    FILE_RDONLY    = (1 << 0),
    FILE_WRONLY    = (1 << 1),
    FILE_RDWR      = (1 << 2),
    FILE_APPEND    = (1 << 3),
    FILE_CREAT     = (1 << 4),
    FILE_DIRECTORY = (1 << 5),
    FILE_DSYNC     = (1 << 6),
    FILE_EXCL      = (1 << 7),
    FILE_NOCTTY    = (1 << 8),
    FILE_NOFOLLOW  = (1 << 9),
    FILE_NONBLOCK  = (1 << 10),
    FILE_RSYNC     = (1 << 11),
    FILE_SYNC      = (1 << 12),
    FILE_TRUNC     = (1 << 13),
};

#if __DPCRT_LINUX
typedef int   pid_t;
typedef int   FileHandle;
typedef pid_t ProcHandle;
typedef void* DllHandle;
/* @NOTE `Invalid_ThreadHandle` is not provided since
   it is not available under UNIX platforms.
   Windows Guarantees that a `HANDLE` type containing 0 means
   an invalid thred handle, but on UNIX the `pthread_t` does
   not reserve any value (it also does not guarantee that `pthread_t` may
   be an integral type. */
typedef U64   ThreadHandle;
typedef pid_t ThreadID;


static const ProcHandle   Invalid_ProcHandle   = -1;
static const FileHandle   Invalid_FileHandle   = -1;
static const FileHandle   Stdin_FileHandle     = 0;
static const FileHandle   Stdout_FileHandle    = 1;
static const FileHandle   Stderr_FileHandle    = 2;
static const DllHandle    Invalid_DllHandle    = 0;


static const ThreadHandle Invalid_ThreadHandle = 0; /* @TODO Is `0` a valid marker for an invalid ThreadHandle on Linux ???? */


#elif __DPCRT_WINDOWS

// @TODO :: Once we start having a nice windows compilation sort those typedefs out.

typedef HFILE   FileHandle;
typedef int     ProcHandle;
typedef HMODULE DllHandle;
typedef HANDLE  ThreadHandle;
typedef DWORD   ThreadID;

static const ProcHandle   Invalid_ProcHandle   = ?????;
static const DllHandle    Invalid_DllHandle    = 0;
static const FileHandle   Invalid_FileHandle   = HFILE_ERROR;
static const FileHandle   Stdin_FileHandle     = 0;
static const FileHandle   Stdout_FileHandle    = 1;
static const FileHandle   Stderr_FileHandle    = 2;
static const DllHandle    Invalid_DllHandle    = 0;
#else
# error "Platform Not Supported"
#endif


typedef struct PAL_Context {
    size_t page_size;
    size_t page_mask;
} PAL_Context;

global PAL_Context G_pal;

#define PAGE_ALIGN(x)       (POW2_ALIGN(size_t, (x), G_pal.page_size))
#define IS_PAGE_ALIGNED(x)  (( (x) % G_pal.page_size ) == 0)


/* Process Termination */
void
pal_exit(int status)
     ATTRIB_NORETURN;

/* Meant for aborting, eg close the process now and do
   not flush any stream. This should used only if you
   know you're in a possibly unrecoverable state. */
void
pal_abort(void)
    ATTRIB_NORETURN;

void
pal_fatal(char *fmt, ...)
    ATTRIB_NORETURN;


void
pal_print_stack_trace(void);


bool
pal_init(void) ATTRIB_CONSTRUCT(pal_init);


/* @NOTE :: This will call into the OS. It
 is advised to access `G_pal.page_size` variable
 instead after a valid call into  `pal_init` is made */
size_t
pal_get_page_size(void);


ThreadID
pal_get_current_thread_id(void);


bool
pal_sleep_ms(U32 sleep_ms);

/* ############## */
/* File */
/* ############## */
FileHandle
pal_openfile(char *path, enum open_file_flags flags);

int
pal_closefile(FileHandle fh);

I64
pal_readfile(FileHandle file, void *buf, I64 size_to_read);

I64
pal_writefile(FileHandle file, void *buf, I64 size_to_write);

/* ########### */
/* Proc */
/* ########### */

ProcHandle
pal_spawnproc_sync ( char *command, int * exit_status );

ProcHandle
pal_spawnproc_async(char *command);


ProcHandle
pal_spawnproc_async_piped ( char *command,
                            FileHandle *inpipe, FileHandle *outpipe);

void
pal_syncproc ( ProcHandle proc, int *status );


int
pal_createdir(char *path);


typedef struct FileTime
{
    time_t tv_sec;  // whole seconds (valid values are >= 0)
    time_t tv_nsec; // nanoseconds (valid values are [0, 999999999 (0.9999.. sec)])
} FileTime;

typedef struct FileLastAccessInfo
{
    FileTime last_access;
    FileTime last_modification;
} FileLastAccessInfo;

bool
pal_get_file_last_access_info(char *filepath, FileLastAccessInfo *out);


/* Return 1 if the difference is negative, otherwise 0. */
int
pal_filetime_diff(FileTime *result,
                  FileTime *x,
                  FileTime *y);

/* This function is a more convenient wrapper around `pal_filetime_diff`
 * which doesn't return the result of the difference */
int
pal_filetime_cmp(FileTime *ft1,
                 FileTime *ft2);


/* ############## */
/* Memory Mapping */
/* ############## */



void*
pal_mmap_file(char *file, void* addr, enum page_prot_flags prot, enum page_type_flags type,
              bool zeroed_page_before, size_t appended_zeroes,
              I64 *buffer_len );
void*
pal_mmap_memory( void* addr, size_t size, enum page_prot_flags prot, enum page_type_flags type );

void*
pal_mremap( void* old_addr, size_t old_size, void *new_addr, size_t new_size,
            enum page_remap_flags flags );

bool                            /* returns was_protected */
pal_mprotect(void *addr, size_t len, enum page_prot_flags prot);

bool
pal_munmap( void* addr, size_t size );


/* Reserves (a possibly huge) chunk of address space memory
   which is not actually committed onto physical memory.
   This is equivalent to `VirtualAlloc` with `MEM_RESERVE` `MEM_COMMIT`
   flags. */
void*  pal_reserve_addr_space   (void *addr, size_t size);
bool   pal_commit_addr_space    (void *addr, size_t size, enum page_prot_flags prot);
bool   pal_uncommit_addr_space  (void *addr, size_t size);
bool   pal_release_addr_space   (void *addr, size_t size);



enum notify_event_flags {
    NotifyEventFlags_None         = 0,
    NotifyEventFlags_Access       = (1 << 0),
    NotifyEventFlags_Attrib       = (1 << 1),
    NotifyEventFlags_CloseWrite   = (1 << 2),
    NotifyEventFlags_CloseNoWrite = (1 << 3),
    NotifyEventFlags_Create       = (1 << 4),
    NotifyEventFlags_Delete       = (1 << 5),
    NotifyEventFlags_DeleteSelf   = (1 << 6),
    NotifyEventFlags_Modify       = (1 << 7),
    NotifyEventFlags_MoveSelf     = (1 << 8),
    NotifyEventFlags_MovedFrom    = (1 << 9),
    NotifyEventFlags_MovedTo      = (1 << 10),
    NotifyEventFlags_Open         = (1 << 11),
};

typedef struct notify_event {
    // If the the current event is valid or just empty (eg no relevant event occured)
    bool valid;

    FileHandle              fh; // The event occured for this specific filehandle
    enum notify_event_flags flags;
} notify_event;

// Non blocking
FileHandle
pal_create_notify_instance(void);

FileHandle
pal_notify_start_watch_file(FileHandle notify_instance_fh,
                            const char *file_path,
                            enum notify_event_flags flags);


void
pal_notify_end_watch_file(FileHandle notify_instance_fh,
                          FileHandle watch_descriptor);


bool
pal_read_notify_event(FileHandle notify_instance_fh,
                      struct notify_event *output);


DllHandle
pal_dll_open(char *path);

void
pal_dll_close(DllHandle handle);

void *
pal_get_proc_addr(DllHandle handle,
                  const char *symbol_name);
















#if _MSC_VER
#  define debug_break __debugbreak
#  define debug_break() __debugbreak
#  define dbg_break()   __debugbreak
#  define dbgbreak()    __debugbreak
#  define __dbgbrk()    __debugbreak
#  define debugger      __debugbreak   /* JS Style Like Debugger break */

#else

# ifndef _SIGNAL_H
/* Forward declare just the stuff that we need from <signal.h> */
   #define SIGTRAP 5
   extern int raise (int sig) ATTRIB_NOTHROW;
#endif

/* Use __builtin_trap() on AArch64 iOS only */

#    if defined(__i386__) || defined(__x86_64__)
__attribute__((gnu_inline, always_inline))
__inline__ static void __trap_instruction(void)
{
    __asm__ volatile("int $0x03");
}

#define debug_break() raise(SIGTRAP)
#define dbg_break()   raise(SIGTRAP)
#define dbgbreak()    raise(SIGTRAP)
#define __dbgbrk()    raise(SIGTRAP)
#define debugger      raise(SIGTRAP)   /* JS Style Like Debugger break */

#    else
#         error "Debug break needs to be written for this architecture"
#    endif
#endif


#undef assert
#undef assert_msg


#if __DEBUG
#    define assert_msg(X, ...)                                          \
    do {                                                                \
        if (!(X))                                                       \
        {                                                               \
            fprintf(stderr, "\n\n################################################################################\n\n"); \
            fprintf(stderr, "ASSERTION FAILED :: [ %s:%d ]\n", __FILE__, __LINE__); \
            fprintf(stderr, "REASON / MESSAGE :: { ");                  \
            fprintf(stderr, __VA_ARGS__);                               \
            fprintf(stderr, " }\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ BACKTRACE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n"); \
            pal_print_stack_trace();                                    \
            fflush(stderr);                                             \
            fprintf(stderr, "\n\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ERRNO INFO ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n"); \
            fprintf(stderr, "* Value of errno: %d\n", errno);           \
            fprintf(stderr, "* Errno desc: %s\n", strerror(errno)); \
            fprintf(stderr, "\n################################################################################\n\n"); \
            fflush(stderr);                                             \
            debug_break();                                              \
        }                                                               \
    } while(0)

#    define assert(X) assert_msg(X, " ")
#else
#    define assert_msg(X, ...)  do { } while (0)
#    define assert(X)           do { } while (0)
#endif


#define assert_range(v, m, M) assert(((v) >= (m)) && ((v) < (M)))


#if __DEBUG
#  define TODO(msg)                do { } while(0)
#  define todo(msg)                TODO(msg)
#  define todo_assert(x)           assert(x)
#  define todo_assert_msg(x, msg)  assert_msg((x), (msg))
#else
#  define TODO(msg)                static_assert(0, "msg :: Please finish all the todo's before releasing a build")
#  define todo(msg)                TODO(msg)
#  define todo_assert(x)           static_assert(0, "Todo asserts are not allowed in release builds. Please provide a proper error handling implementation")
#  define todo_assert_msg(x, msg)  static_assert(0, msg " :: " "Todo asserts are not allowed in release builds. Please provide a proper error handling implementation")
#endif


#define invalid_code_path(...)  assert_msg ( 0, "Invalid code path" )
#define not_implemented(...)    assert_msg ( 0, "Not implemented code path" )



#if __DEBUG

#include "dpcrt_atomics.h"


/* This macro can be usefull in debug Builds to mark a function to be `non_thread_safe`.
   If the function is called from multiple threads, this macro will assert.
   The first thread that calls a function marked with `non_thread_safe()` will
   own the function from now on. Any other thread trying to reuse the function
   from now on will trigger an assertion.
   This can be usefull in functions where you use global variables or static variables,
   or even in callback functions arriving from a 3rd party library to
   assert and make sure that you always get called from the same thread. */
#  define non_thread_safe()                                             \
    do {                                                                \
        static ThreadID __last_thread_id__ = 0;                         \
        ThreadID __expected__ = 0;                                      \
        ThreadID id = pal_get_current_thread_id();                      \
        if (!atomic_compare_exchange(&__last_thread_id__, &__expected__, id)) \
        {                                                               \
            assert_msg(__expected__ == id,                              \
                       "This thread does not own this function");       \
        }                                                               \
    } while(0)

#else
#  define non_thread_safe()
#endif




str32_list_t get_files_in_dir(char *dirpath, miface_t *allocator);

str32_t get_file_root(str32_t filepath, miface_t *allocator);
str32_t get_file_ext(str32_t filepath, miface_t *allocator);

str32_t get_dirname(str32_t filepath, miface_t *allocator);
str32_t get_basename(str32_t filepath, miface_t *allocator);

path_t path_from_str32(str32_t path, miface_t *allocator);
str32_t path_to_str32(path_t path, miface_t *allocator);

void os_sleep(double secs);

bool os_mkdir(char *filepath);
void os_mkdir_p(miface_t *temp_allocator, char *path);

void os_rmdir(miface_t *temp_allocator, char *path);
void os_rmdir_contents(miface_t *temp_allocator, char *path);

bool os_fexists(char *path);

double os_get_ms(void);

void os_print_stacktrace(void);
