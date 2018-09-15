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

#if __linux__
#  include <errno.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <signal.h>
#  include <sys/wait.h>
#  include <sys/inotify.h>
#  include <dlfcn.h>
#  define MAP_HUGE_2MB    (21 << MAP_HUGE_SHIFT)
#  define MAP_HUGE_1GB    (30 << MAP_HUGE_SHIFT)
#elif __WINDOWS__
#  include <windows.h>
#endif




#ifdef __linux__
#  include <execinfo.h>
#  include <stdlib.h>


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


void
linux_print_stack_trace(void)
{
    void *array[32];
    size_t size;

    fprintf(stderr, "Printing stack trace: #####\n");
    // get void*'s for all entries on the stack
    size = backtrace(array, ARRAY_LEN(array));
    // print out all the frames to stderr

    backtrace_symbols_fd(array, size, STDERR_FILENO);
    fprintf(stderr, "########################\n");
    fflush(stderr);
}


#if defined __linux__ && defined __DEBUG

void gdb_print_stack_trace(void) {
    char pid_buf[30];
    snprintf(pid_buf, sizeof(pid_buf), "%d", getpid());
    char name_buf[512];
    name_buf[readlink("/proc/self/exe", name_buf, 511)]=0;
    int child_pid = fork();
    if (!child_pid) {
        dup2(2,1); // redirect output to stderr
        fprintf(stderr,"stack trace for %s pid=%s\n",name_buf,pid_buf);
        execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt full", name_buf, pid_buf, NULL);
        abort(); /* If gdb failed to start */
    } else {
        waitpid(child_pid,NULL,0);
    }
}

#endif


void
pal_print_stack_trace(void)
{
#if __linux__ && __DEBUG
    gdb_print_stack_trace();
#elif __linux__
    linux_print_stack_trace();
#else
#  error "Needs implementation"
#endif

}

static void sigsegv_linux_callback(int sig)
{
    fprintf(stderr, "Error: signal %d: %s\n", sig, strsignal(sig));
    pal_print_stack_trace();
    exit(-1);
}
#endif


int64_t
pal_get_page_size( void )
{
#if __linux__
    static int64_t cached_page_size = 0;
    if ( cached_page_size == 0 ) {
        int64_t page_size = sysconf(_SC_PAGESIZE);
        assert( page_size );
        cached_page_size = page_size;
        return page_size;
    } else {
        return cached_page_size;
    }
#elif __WINDOWS__
#    error "Needs implementation for Windows"
#else
#    error "Not supported platform"
#endif
}

bool
pal_sleep_ms(uint32 sleep_ms)
{
    bool success = true;
#if __linux__

    useconds_t usec = (useconds_t) sleep_ms * (useconds_t) 1000;
    int usleep_result = usleep(usec);
    if ( usleep_result != 0 )
    {
        success = false;
    }

#elif __WINDOWS__
#    error "Needs implementation for Windows"
#else
#    error "Not supported platform"
#endif
        return success;
}




static inline int
file_flags__convert(enum open_file_flags flags)
{
#ifdef __linux__
    int result = 0;
    if (flags & FILE_RDONLY) {
        result |= O_RDONLY;
    }
    if (flags & FILE_WRONLY) {
        result |= O_WRONLY;
    }
    if (flags & FILE_RDWR) {
        result |= O_RDWR;
    }
    if (flags & FILE_APPEND) {
        result |= O_APPEND;
    }
    if (flags & FILE_CREAT) {
        result |= O_CREAT;
    }
    if (flags & FILE_DIRECTORY) {
        result |= O_DIRECTORY;
    }
    if (flags & FILE_DSYNC) {
        result |= O_DSYNC;
    }
    if (flags & FILE_EXCL) {
        result |= O_EXCL;
    }
    if (flags & FILE_NOCTTY) {
        debug_break();
        result |= O_NOCTTY;
    }
    if (flags & FILE_NOFOLLOW) {
        result |= O_NOFOLLOW;
    }
    if (flags & FILE_NONBLOCK) {
        result |= O_NONBLOCK;
    }
    if (flags & FILE_RSYNC) {
        result |= O_RSYNC;
    }
    if (flags & FILE_SYNC) {
        result |= O_SYNC;
    }
    if (flags & FILE_TRUNC) {
        result |= O_TRUNC;
    }
    return result;
#else
#   error "Not supported platform needs implementation"
#endif
}


filehandle_t
pal_openfile(char *path, enum open_file_flags flags)
{
#ifdef __linux__
    {
        int linux_flags = file_flags__convert(flags);
        filehandle_t result = open(path, linux_flags, S_IXUSR | S_IXGRP |S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        return result;
    }
#else
#    error "Needs implementation"
#endif

}

int
pal_closefile(filehandle_t fh)
{
#ifdef __linux__
    {
        // returns -1 on failure
        return close(fh);
    }
#else
#    error "Needs implementation"
#endif

}

static inline int
prot_flags__convert(enum page_prot_flags prot)
{
#ifdef __linux__
    int result = 0;
    if ( prot & PAGE_PROT_READ ) {
        result |= PROT_READ;
    }
    if (prot & PAGE_PROT_WRITE ) {
        result |= PROT_WRITE;
    }
    if (prot & PAGE_PROT_EXEC ) {
        result |= PROT_EXEC;
    }
    return result;
#else
#   error "Not supported platform needs implementation"
#endif
}


static inline int
page_type_flags__convert(enum page_type_flags type)
{
#ifdef __linux__

    int result = 0;
    if ( type & PAGE_FIXED ) {
        result |= MAP_FIXED;
    }
    if ( type & PAGE_PRIVATE ) {
        result |= MAP_PRIVATE;
    }
    if ( type & PAGE_ANONYMOUS ) {
        result |= MAP_ANONYMOUS;
    }
    if ( type & PAGE_LOCKED ) {
        result |= MAP_LOCKED;
    }
    if ( type & PAGE_GROWSDOWN ) {
        result |= MAP_GROWSDOWN;
    }
    if ( type & PAGE_POPULATE ) {
        result |= MAP_POPULATE;
    }
    if ( type & PAGE_HUGETLB ) {
        result |= MAP_HUGETLB;
    }
    if ( type & PAGE_HUGE_2MB ) {
        result |= MAP_HUGE_2MB;
    }
    if ( type & PAGE_HUGE_1GB ) {
        result |= MAP_HUGE_1GB;
    }
    return result;
#else
#   error "Not supported platform needs implementation"
#endif
}

// Insipired by mmap function, auxiliary functions
//     which other specilized functions depends on
static void*
pal_mmap_aux( void* addr, size_t size, enum page_prot_flags prot, enum page_type_flags type, filehandle_t fh)
{
#if __linux__
    void *result = 0x0000;
    int linux_prot  = prot_flags__convert(prot);
    int linux_flags = page_type_flags__convert(type);
    int32_t offset = 0;
    result = mmap( addr, size, linux_prot, linux_flags, fh, offset);
    if (result == MAP_FAILED ) {
#if __DEBUG
        {
            int errno_val = errno;
            (void) errno_val;
            char *error_message = strerror(errno_val);
            (void) error_message;
        }
#endif
        result = 0;
    }
    return result;
#elif __WINDOWS__
#    error "Needs windows implementation"
#else
#    error "Other platform needs implementation"
#endif
}

void*
pal_mmap_memory( void* addr, size_t size, enum page_prot_flags prot, enum page_type_flags type )
{
    const filehandle_t fh = 0;
    return pal_mmap_aux( addr, size, prot, type, fh );
}


static inline int
page_remap_flags__convert(enum page_remap_flags flags)
{
#ifdef __linux__

    int result = 0;
    if ( flags & PAGE_REMAP_MAYMOVE ) {
        result |= MREMAP_MAYMOVE;
    }
    if ( flags & PAGE_REMAP_FIXED ) {
        result |= MREMAP_FIXED;
    }
    return result;
#else
#   error "Not supported platform needs implementation"
#endif
}

// new addr can be NULL if you don't care where its going to be remapped
void*
pal_mremap( void* old_addr, size_t old_size, void *new_addr, size_t new_size,
            enum page_remap_flags flags )
{
#ifdef __linux__
    {
        void *result = NULL;
        int linux_flags = page_remap_flags__convert(flags);
        if ( new_addr ) {
            result = mremap(old_addr, old_size, new_size, linux_flags, new_addr);
        }
        else {
            result = mremap(old_addr, old_size, new_size, linux_flags );
            if ( result == MAP_FAILED) {
                result = 0;
            }
        }
        assert(result);
        return result;
    }
#else
#error "Needs implementation for current platform"
#endif
}



bool                            /* returns was_protected */
pal_mprotect(void *addr, size_t len, enum page_prot_flags prot)
{
#ifdef __linux__
    {
        int linux_prot = prot_flags__convert(prot);
        int result     = mprotect(addr, len, linux_prot);
        if ( result )
        {
            char *err_message = strerror(errno);
            fprintf(stderr, "pal_mprotect() :: %s\n", err_message);
            return false;
        }
        else
        {
            return true;
        }
    }
#else
#  error "Needs implementation for current platform"
#endif
}




// addr is usually null
void*
pal_mmap_file(char *file, void* addr, enum page_prot_flags prot, enum page_type_flags type,
              bool zeroed_page_before, size_t appended_zeroes,
              int64_t *buffer_len ) // Output: The buffer len (eg the length of the file)
{
#if __linux__
    void *result = 0x0000;
    filehandle_t fh = pal_openfile(file, FILE_RDWR);
    if ( fh == invalid_filehandle ) {
        return NULL;
    }
    assert(fh != invalid_filehandle);
    struct stat fdstat;
    int st = stat(file, &fdstat);
    assert(st == 0);
    int64_t page_size = pal_get_page_size();
    uint8_t *zeroed_page = 0x0000;

    if ( zeroed_page_before ) {
        zeroed_page = pal_mmap_aux( addr, page_size + fdstat.st_size + appended_zeroes,
                                    prot, type, 0 );
        assert(zeroed_page);
    }
    void *newaddr = addr;
    enum page_type_flags ptype = type;
    ptype = ptype & (~PAGE_ANONYMOUS);
    if ( zeroed_page_before ) {
        newaddr = zeroed_page + page_size;
        ptype |= PAGE_FIXED;
    }
    result = pal_mmap_aux ( newaddr, fdstat.st_size + appended_zeroes,
                            prot, ptype, fh );

    assert(result);
    if ( buffer_len ) {
        *buffer_len = fdstat.st_size + appended_zeroes;
    }

    pal_closefile(fh);
    return result;
#else
#   error "Not supported platform needs implementation"
#endif
}



int
pal_munmap( void* addr, size_t size )
{
#if __linux__
    int result = 0; // 0 = success
    result = munmap(addr, size);
    return result;
#else
#   error "Needs implementation"
#endif
}


int
pal_init( void )
{
#ifdef __linux__
    {
        int result = false;

        { // Page Size retrieve
            signal(SIGSEGV, sigsegv_linux_callback);   // install our handler
        }

        return result;
    }
#elif __WINDOWS__
#    error "Implement me for Windows please"
#else
#    error "Not supported platform"
#endif
}

int
pal_createdir(char *path )
{
#ifdef __linux__
    return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#else
#   error "Not supported platform"
#endif
}


bool
pal_stat_filetime(char *filepath, struct stat_filetime *out)
{
    bool success = true;
#ifdef __linux__

    struct stat statbuf;
    int s = stat(filepath, & statbuf);
    if (s != 0)
    {
        success = false;
        memclr(out, sizeof(*out));
    }
    else
    {
        out->st_atim = statbuf.st_atim;
        out->st_mtim = statbuf.st_mtim;
        out->st_ctim = statbuf.st_ctim;
    }

#elif __WINDOWS__

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
            & (out->creationTime)
            & (out->lastAccessTime),
            & (out->lastWriteTime));

        if (b == 0)
        {
            memclr(out, sizeof(*out));
            success = false;
        }

        b = CloseHandle(handle);
        assert(b != 0);
    }
#else
#    error "Not supported platform, needs implementation"
#endif
    return success;
}


/* Subtract the ‘struct timeval’ values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */
int
linux__struct_timespec_diff (struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}


/* Subtract the ‘struct timeval’ values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */
int
linux__struct_timeval_diff (struct timespec *result, struct timespec *x, struct timespec *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_nsec < y->tv_nsec) {
    int nsec = (y->tv_nsec - x->tv_nsec) / 1000000000ll + 1;
    y->tv_nsec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_nsec - y->tv_nsec > 1000000) {
    int nsec = (x->tv_nsec - y->tv_nsec) / 1000000000ll;
    y->tv_nsec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_nsec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_nsec = x->tv_nsec - y->tv_nsec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}



int
pal_filetime_cmp(struct filetime *ft1,
                 struct filetime *ft2)
{
#ifdef __linux__
    struct timespec diff;

    int result = 0;
    if ((ft1->ts.tv_sec == ft2->ts.tv_sec)
        && (ft1->ts.tv_nsec == ft2->ts.tv_nsec))
    {
        result = 0;
    }
    else
    {
        int is_negative = linux__struct_timeval_diff (& diff, & ft1->ts, & ft2->ts);
        result = is_negative ? -1 : 1;
    }

    return result;

#elif __WINDOWS__
    LONG l = CompareFileTime(
        & (ft1->ft),
        & (ft2->ft)
        );
    return (int) l;
#else
#    error "Not supported platform, needs implementation"
#endif

}





int64_t
pal_readfile(filehandle_t file, void *buf, int64_t size_to_read)
{
#ifdef __linux__
    {
        int64_t result = read(file, buf, size_to_read);
#if __DEBUG
        if (result < 0)
        {
            // Place a breakpoint here for convenience to see the error message if any
            char *err_message = strerror(errno);
            int breakme;
        }
#endif
        return result;
    }
#else
#    error "Not supported platform, needs implementation"
#endif

}


int64_t
pal_writefile(filehandle_t file, void *buf, int64_t size_to_write)
{
#ifdef __linux__
    {
        int64_t result = write(file, buf, size_to_write);
#if __DEBUG
        if (result < 0)
        {
            // Place a breakpoint here for convenience to see the error message if any
            char *err_message = strerror(errno);
            int breakme;
        }
#endif
        return result;
    }
#else
#    error "Not supported platform needs implementation"
#endif

}



prochandle_t
pal_spawnproc_sync ( char *command, int * exit_status )
{
#ifdef __linux__
    pid_t id = fork ();
    if ( id == - 1) {
        // error on fork
    }
    else if ( id == 0 ) {
        // child process
        if (execl("/bin/sh", "sh", "-c", command, (char *) 0) ) {
            // DOH Make me crash
            *((char*)0) = 0;
        }
    }
    else  {
        // parent process
        // wait for child to complete
        waitpid(-1, exit_status, 0);
    }
    return id;
#elif __WINDOWS__
# error "Not supported platform needs implementation"
#else
# error "Not supported platform needs implementation"
#endif
}




prochandle_t
pal_spawnproc_async ( char * command )
{
#ifdef __linux__
    pid_t id = fork ();
    if ( id == - 1) {
        // error on fork
    }
    else if ( id == 0 ) {
        // child process
        if ( execl("/bin/sh", "sh", "-c", command, (char *) 0) ) {
            // DOH Make me crash

            *((char*)0) = 0;
        }
    }
    else  {
        // parent process
    }
    return id;
#else
#    error "Not supported platform needs implementation"
#endif
}


prochandle_t
pal_spawnproc_async_piped ( char *command,
                            filehandle_t *inpipe, filehandle_t *outpipe)
{
#ifdef __linux__
    filehandle_t outpipes[2] = {0};
    filehandle_t inpipes[2] = {0};
    int piperes = pipe(outpipes);
    UNUSED(piperes);
    assert(piperes == 0);
    piperes = pipe(inpipes);
    assert(piperes == 0 );
    pid_t pid = fork ();
    if ( pid == -1) {
        // error on fork
        assert_msg(0, "Failed to fork the requested proc");
    }
    else if ( pid == 0 ) {
        // child process
        if (inpipe) {
            dup2(inpipes[0], STDIN_FILENO);
        }
        close(inpipes[0]);
        close(inpipes[1]);
        if (outpipe) {
            dup2(outpipes[1], STDOUT_FILENO);
        }
        close(outpipes[0]);
        close(outpipes[1]);
        if (execl("/bin/bash", "bash", "-c", command, (char *) 0) ) {
            // DOH Make me crash
            *((char*)0) = 0;
        }
    }
    else  {
        close(inpipes[0]);
        close(outpipes[1]);
        if (inpipe) {
            *inpipe = inpipes[1];
        } else {
            close(inpipes[1]);
        }
        if (outpipe) {
            *outpipe = outpipes[0];
        } else {
            close(outpipes[0]);
        }
    }
    return pid;
#elif __WINDOWS__
# error  "Not supported platform needs implementation"
#else
# error "Not supported platform needs implementation"
#endif
}


void
pal_syncproc ( prochandle_t proc, int *status )
{
#ifdef __linux__
    int local_status;
    waitpid ( proc, (status != NULL) ? status : &local_status, 0 );
#else
#   error "Not supported platform needs implementation"
#endif
}



filehandle_t
pal_create_notify_instance(void)
{
#ifdef __linux__
    return inotify_init1(IN_NONBLOCK);
#else
#   error "Not supported platform needs implementation"
#endif

}

static inline uint32
notify_event_flags__convert(enum notify_event_flags flags)
{
#ifdef __linux__
    uint32 result = 0;
    if (flags & NotifyEventFlags_Access) {
        result |= IN_ACCESS;
    }
    if (flags & NotifyEventFlags_Attrib) {
        result |= IN_ATTRIB;
    }
    if (flags & NotifyEventFlags_CloseWrite) {
        result |= IN_CLOSE_WRITE;
    }
    if (flags & NotifyEventFlags_CloseNoWrite) {
        result |= IN_CLOSE_NOWRITE;
    }
    if (flags & NotifyEventFlags_Create) {
        result |= IN_CREATE;
    }
    if (flags & NotifyEventFlags_Delete) {
        result |= IN_DELETE;
    }
    if (flags & NotifyEventFlags_DeleteSelf) {
        result |= IN_DELETE_SELF;
    }
    if (flags & NotifyEventFlags_Modify) {
        result |= IN_MODIFY;
    }
    if (flags & NotifyEventFlags_MoveSelf) {
        result |= IN_MOVE_SELF;
    }
    if (flags & NotifyEventFlags_MovedFrom) {
        result |= IN_MOVED_FROM;
    }
    if (flags & NotifyEventFlags_MovedTo) {
        result |= IN_MOVED_TO;
    }
    if (flags & NotifyEventFlags_Open) {
        result |= IN_OPEN;
    }

    return result;
#else
#   error "Not supported platform needs implementation"
#endif
}

#ifdef __linux__
static inline enum notify_event_flags
linux_inotify_event_mask__convert(uint32_t mask)
{
    enum notify_event_flags result = 0;
    if (mask & IN_ACCESS) {
        result |= NotifyEventFlags_Access;
    }
    if (mask & IN_ATTRIB) {
        result |= NotifyEventFlags_Attrib;
    }
    if (mask & IN_CLOSE_WRITE) {
        result |= NotifyEventFlags_CloseWrite;
    }
    if (mask & IN_CLOSE_NOWRITE) {
        result |= NotifyEventFlags_CloseNoWrite;
    }
    if (mask & IN_CREATE) {
        result |= NotifyEventFlags_Create;
    }
    if (mask & IN_DELETE) {
        result |= NotifyEventFlags_Delete;
    }
    if (mask & IN_DELETE_SELF) {
        result |= NotifyEventFlags_DeleteSelf;
    }
    if (mask & IN_MODIFY) {
        result |= NotifyEventFlags_Modify;
    }
    if (mask & IN_MOVE_SELF) {
        result |= NotifyEventFlags_MoveSelf;
    }
    if (mask & IN_MOVED_FROM) {
        result |= NotifyEventFlags_MovedFrom;
    }
    if (mask & IN_MOVED_TO) {
        result |= NotifyEventFlags_MovedTo;
    }
    if (mask & IN_OPEN) {
        result |= NotifyEventFlags_Open;
    }
    return result;
}
#endif


filehandle_t
pal_notify_start_watch_file(filehandle_t notify_instance_fh,
                            const char *file_path,
                            enum notify_event_flags flags)
{
    assert(notify_instance_fh != invalid_filehandle);
#ifdef __linux__
    uint32 mask = notify_event_flags__convert(flags);
    filehandle_t result = inotify_add_watch(notify_instance_fh, file_path, mask);
    if (result != invalid_filehandle)
    {
        int r = fcntl(result, F_SETFL, fcntl(result, F_GETFL, 0) | O_NONBLOCK);
        if (r == -1)
        {
            fprintf(stderr, "Failed to set NONBLOCK mode for `inotify` component\n");
        }
    }
    return result;
#else
#   error "Not supported platform needs implementation"
#endif

}

void
pal_notify_end_watch_file(filehandle_t notify_instance_fh,
                          filehandle_t watch_descriptor)
{

    assert(notify_instance_fh && notify_instance_fh != invalid_filehandle);
    assert(watch_descriptor && watch_descriptor != invalid_filehandle);
#ifdef __linux__
    inotify_rm_watch(notify_instance_fh, watch_descriptor);
#else
#   error "Not supported platform needs implementation"
#endif

}



bool
pal_read_notify_event(filehandle_t notify_instance_fh,
                      struct notify_event *output)
{

#ifdef __linux__
    bool success = true;
    struct inotify_event e = {0};

    ssize_t size = read(notify_instance_fh, & e, sizeof(e));

    if (size > 0)
    {
        output->valid = true;
        output->fh = e.wd;
        output->flags = linux_inotify_event_mask__convert(e.mask);
    }
    else if ( size == -1 && errno == EAGAIN)
    {
        memclr(output, sizeof(*output));
    }
    else
    {
        memclr(output, sizeof(*output));
        success = false;
    }
    return success;
#else
#   error "Not supported platform needs implementation"
#endif

}



dll_handle_t
pal_dll_open(char *path)
{
    dll_handle_t result = 0;
#ifdef __linux__
    result = dlopen(path, RTLD_NOW | RTLD_LOCAL);
#elif __WINDOWS__
    result = LoadLibrary(path);
#else
#   error "Not supported platform needs implementation"
#endif

    return result;
}

void
pal_dll_close(dll_handle_t handle)
{
    assert(handle != invalid_dll_handle);
#ifdef __linux__

    int dlres = dlclose(handle);
    (void) dlres;
    assert(dlres == 0);
#elif __WINDOWS__
    result = LoadLibrary(path);
#else
#   error "Not supported platform needs implementation"
#endif

}

void *
pal_get_proc_addr(dll_handle_t handle,
                  const char *symbol_name)
{
    void *result = NULL;
    assert(handle != invalid_dll_handle);
#ifdef __linux__
    result = dlsym(handle, symbol_name);
#elif __WINDOWS__
    result = (void*) GetProcAddress(handle, symbol_name);
#else
#   error "Not supported platform needs implementation"
#endif
    return result;
}
