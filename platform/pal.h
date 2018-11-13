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
#ifndef PLATFORM__PAL_H
#define PLATFORM__PAL_H

#include "utils.h"
#include <stdio.h>

__BEGIN_DECLS


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

#if __PAL_LINUX__
typedef int pid_t;
typedef int   filehandle_t;
typedef pid_t prochandle_t;
typedef void* dll_handle_t;

static const prochandle_t null_prochandle    = -1; /* same thing as invalid_prochandle_t, matter of taste/style */
static const prochandle_t invalid_prochandle = -1;
static const filehandle_t invalid_filehandle = -1;
static const filehandle_t stdin_filehandle   = 0;
static const filehandle_t stdout_filehandle  = 1;
static const filehandle_t stderr_filehandle  = 2;
static const dll_handle_t null_dll_handle = 0;
static const dll_handle_t invalid_dll_handle = 0;


#elif __PAL_WINDOWS__

// @TODO :: Once we start having a nice windows compilation sort those typedefs out.

typedef HFILE   filehandle_t;
typedef int     prochandle_t;
typedef HMODULE dll_handle_t;

static const filehandle_t null_filehandle    = HFILE_ERROR; /* Same thing as Invalid_filehandle_t, matter of taste/style */
static const filehandle_t invalid_filehandle = HFILE_ERROR;
static const filehandle_t stdin_filehandle   = 0;
static const filehandle_t stdout_filehandle  = 1;
static const filehandle_t stderr_filehandle  = 2;
static const dll_handle_t null_dll_handle = 0;
static const dll_handle_t invalid_dll_handle = 0;
#else
# error "Platform Not Supported"
#endif


filehandle_t filehandle;


void
fatal(char *fmt, ...);



void
pal_print_stack_trace(void);

int
pal_init(void);

int64_t
pal_get_page_size(void);


bool
pal_sleep_ms(uint32 sleep_ms);

/* ############## */
/* File */
/* ############## */
filehandle_t
pal_openfile(char *path, enum open_file_flags flags);

int
pal_closefile(filehandle_t fh);

int64_t
pal_readfile(filehandle_t file, void *buf, int64_t size_to_read);

int64_t
pal_writefile(filehandle_t file, void *buf, int64_t size_to_write);

/* ########### */
/* Proc */
/* ########### */

prochandle_t
pal_spawnproc_sync ( char *command, int * exit_status );

prochandle_t
pal_spawnproc_async(char *command);


prochandle_t
pal_spawnproc_async_piped ( char *command,
                            filehandle_t *inpipe, filehandle_t *outpipe);

void
pal_syncproc ( prochandle_t proc, int *status );


int
pal_createdir(char *path);


typedef struct filetime
{
    time_t tv_sec;  // whole seconds (valid values are >= 0) 
    time_t tv_nsec; // nanoseconds (valid values are [0, 999999999 (0.9999.. sec)])
} filetime_t;

typedef struct stat_filetime
{
    struct filetime last_access;
    struct filetime last_modification;
} stat_filetime_t;

bool
pal_stat_filetime(char *filepath, struct stat_filetime *out);


/* Return 1 if the difference is negative, otherwise 0. */
int
pal_filetime_diff(struct filetime *result,
                  struct filetime *x,
                  struct filetime *y);

/* This function is a more convenient wrapper around `pal_filetime_diff` 
 * which doesn't return the result of the difference */
int
pal_filetime_cmp(struct filetime *ft1,
                 struct filetime *ft2);


/* ############## */
/* Memory Mapping */
/* ############## */

void*
pal_mmap_file(char *file, void* addr, enum page_prot_flags prot, enum page_type_flags type,
              bool zeroed_page_before, size_t appended_zeroes,
              int64_t *buffer_len );
void*
pal_mmap_memory( void* addr, size_t size, enum page_prot_flags prot, enum page_type_flags type );

void*
pal_mremap( void* old_addr, size_t old_size, void *new_addr, size_t new_size,
            enum page_remap_flags flags );

bool                            /* returns was_protected */
pal_mprotect(void *addr, size_t len, enum page_prot_flags prot);

int
pal_munmap( void* addr, size_t size );


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

    filehandle_t             fh; // The event occured for this specific filehandle
    enum notify_event_flags  flags;
} notify_event;

// Non blocking
filehandle_t
pal_create_notify_instance(void);

filehandle_t
pal_notify_start_watch_file(filehandle_t notify_instance_fh,
                            const char *file_path,
                            enum notify_event_flags flags);


void
pal_notify_end_watch_file(filehandle_t notify_instance_fh,
                          filehandle_t watch_descriptor);


bool
pal_read_notify_event(filehandle_t notify_instance_fh,
                      struct notify_event *output);


dll_handle_t
pal_dll_open(char *path);

void
pal_dll_close(dll_handle_t handle);

void *
pal_get_proc_addr(dll_handle_t handle,
                  const char *symbol_name);
















#if _MSC_VER
#     define debug_break __debugbreak
#else
#    include <signal.h>

/* Use __builtin_trap() on AArch64 iOS only */

#    if defined(__i386__) || defined(__x86_64__)
__attribute__((gnu_inline, always_inline))
__inline__ static void trap_instruction(void)
{
    __asm__ volatile("int $0x03");
}
__attribute__((gnu_inline, always_inline))
__inline__ static void __debug_break(char* file, int32_t line)
{
    fprintf(stderr, "\n\n\nCall to debug_break || file: %s || line: %d\n", file, line);
    fprintf(stderr, "Previous value of errno: %d\n", errno);
    fprintf(stderr, "    errno desc: %s\n\n\n", strerror(errno));
    fflush(stderr);
    raise(SIGTRAP);
}

#define debug_break() __debug_break( __FILE__, __LINE__)

#    else
#         error "Debug break needs to be written for this architecture"
#    endif
#endif


#define log_message(...)        do { fprintf(stderr, __VA_ARGS__); } while (0)
#define dbglog(...)             do { fprintf(stderr, __VA_ARGS__); } while (0)



#undef assert
#undef assert_msg
#undef static_assert

#if __DEBUG
#    define assert_msg(X, ...)                  \
    do {                                        \
        if (!(X)) {                             \
            fprintf(stderr, "\n\n\n");          \
            fprintf(stderr, __VA_ARGS__);       \
            fprintf(stderr, "\n\n");            \
            pal_print_stack_trace();            \
            fflush(stderr);                     \
            debug_break();                      \
        }                                       \
    } while(0)

#    define assert(X) assert_msg(X, "Failed assertion at file: %s || line: %d\n", __FILE__, __LINE__)
#else
#    define assert_msg(X, ...)                  \
    do {                                        \
    } while(0)

#    define assert(X) do { } while(0)
#endif

#ifndef static_assert
# if __STDC_VERSION__ >= STD_C11_VERSION
#  define static_assert(cond, msg) _Static_assert((cond), msg)
# else
#  define static_assert(cond, msg) struct CONCAT(___static_assertion___, __LINE__ ) { int ___compile_time_assertion_failed___[!cond ? -1 : 0]; }
# endif
#endif


#if defined __DEBUG
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


#define invalid_code_path(...)  assert_msg ( 0, "Invalid code path || file: %s || line: %d", __FILE__, __LINE__ )
#define not_implemented(...)    assert_msg ( 0, "Not implemented code path || file: %s || line: %d", __FILE__, __LINE__ )





__END_DECLS

#endif  /* PLATFORM__PAL_H */
