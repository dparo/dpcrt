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

#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/inotify.h>
#include <dlfcn.h>
#define MAP_HUGE_2MB    (21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB    (30 << MAP_HUGE_SHIFT)

#include <execinfo.h>
#include <stdlib.h>
#include <sys/types.h>

void
pal_exit(int status)
{
    _exit(status);
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
linux_print_stack_trace(void)
{
    void *array[32];
    int size;

    fprintf(stderr, "Printing stack trace: #####\n");
    // get void*'s for all entries on the stack
    size = backtrace(array, ARRAY_LEN(array));
    // print out all the frames to stderr

    backtrace_symbols_fd(array, size, STDERR_FILENO);
    fprintf(stderr, "########################\n");
    fflush(stderr);
}

static void
gdb_print_stack_trace(void)
{
    char name_buf[512]; 
    char pid_buf[30];
    snprintf(pid_buf, sizeof(pid_buf), "%d", getpid());
    ssize_t proc_self_exe_len = readlink("/proc/self/exe", name_buf, sizeof(name_buf) - 1);

    if (proc_self_exe_len == -1)
    {
         linux_print_stack_trace();
    }
    else
    {
        name_buf[ proc_self_exe_len ] = 0;
        int child_pid = fork();
        if (!child_pid)
        {
            dup2(2, 1); // redirect output to stderr
            fprintf(stderr,"stack trace for %s pid=%s\n", name_buf, pid_buf);
            execlp("gdb", "gdb", "--batch", "-n", "-ex", "thread", "-ex", "bt full", name_buf, pid_buf, NULL);
            /* execlp does not return; if, for some reason, gdb fails to start revert back to a simple stack_trace print */
            {
                linux_print_stack_trace();
                abort();
            }
        }
        else
        {
            /* If gdb started wait for it. */
            waitpid(child_pid, NULL, 0);
        }
    }
}

void
pal_print_stack_trace(void)
{
#if __DEBUG
    gdb_print_stack_trace();
#else
    linux_print_stack_trace();
#endif
}

static void
sigsegv_linux_callback(int sig)
{
    fprintf(stderr, "Error: signal %d: %s\n", sig, strsignal(sig));
    pal_print_stack_trace();
    exit(-1);
}


I64
pal_get_page_size( void )
{
    static I64 cached_page_size = 0;
    if ( cached_page_size == 0 ) {
        I64 page_size = sysconf(_SC_PAGESIZE);
        assert( page_size );
        cached_page_size = page_size;
        return page_size;
    } else {
        return cached_page_size;
    }
}

bool
pal_sleep_ms(U32 sleep_ms)
{
    bool success = true;
    useconds_t usec = (useconds_t) sleep_ms * (useconds_t) 1000;
    int usleep_result = usleep(usec);
    if ( usleep_result != 0 )
    {
        success = false;
    }
    return success;
}




static inline int
file_flags__convert(enum open_file_flags flags)
{
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
}


FileHandle
pal_openfile(char *path, enum open_file_flags flags)
{
    int linux_flags = file_flags__convert(flags);
    FileHandle result = open(path, linux_flags, S_IXUSR | S_IXGRP |S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    return result;
}

int
pal_closefile(FileHandle fh)
{
    // returns -1 on failure
    return close(fh);
}

static inline int
prot_flags__convert(enum page_prot_flags prot)
{
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
}


static inline int
page_type_flags__convert(enum page_type_flags type)
{
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
}

// Insipired by mmap function, auxiliary functions
//     which other specilized functions depends on
static void*
pal_mmap_aux( void* addr, size_t size, enum page_prot_flags prot, enum page_type_flags type, FileHandle fh)
{
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
}

void*
pal_mmap_memory( void* addr, size_t size, enum page_prot_flags prot, enum page_type_flags type )
{
    const FileHandle fh = 0;
    return pal_mmap_aux( addr, size, prot, type, fh );
}


static inline int
page_remap_flags__convert(enum page_remap_flags flags)
{
    int result = 0;
    if ( flags & PAGE_REMAP_MAYMOVE ) {
        result |= MREMAP_MAYMOVE;
    }
    if ( flags & PAGE_REMAP_FIXED ) {
        result |= MREMAP_FIXED;
    }
    return result;
}

// new addr can be NULL if you don't care where its going to be remapped
void*
pal_mremap( void* old_addr, size_t old_size, void *new_addr, size_t new_size,
            enum page_remap_flags flags )
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



bool                            /* returns was_protected */
pal_mprotect(void *addr, size_t len, enum page_prot_flags prot)
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




// addr is usually null
void*
pal_mmap_file(char *file, void* addr, enum page_prot_flags prot, enum page_type_flags type,
              bool zeroed_page_before, size_t appended_zeroes,
              I64 *buffer_len ) // Output: The buffer len (eg the length of the file)
{
    void* result = 0;
    FileHandle fh = pal_openfile(file, FILE_RDWR);
    if ( fh == Invalid_FileHandle )
    {
        return NULL;
    }
    assert(fh != Invalid_FileHandle);
    struct stat fdstat;
    int st = stat(file, &fdstat);
    assert(st == 0);
    if (st == 0)
    {
        I64 page_size = pal_get_page_size();
        void *zeroed_page = 0;

        if ( zeroed_page_before )
        {
            zeroed_page = pal_mmap_aux( addr, (size_t) page_size + (size_t) fdstat.st_size + appended_zeroes,
                                        prot, type, 0 );
            assert(zeroed_page);
        }
        void *newaddr = addr;
        enum page_type_flags ptype = type;
        ptype = ptype & (~((U32) PAGE_ANONYMOUS));
        if ( zeroed_page_before )
        {
            newaddr = zeroed_page + page_size;
            ptype |= PAGE_FIXED;
        }
        result = pal_mmap_aux ( newaddr, (size_t) fdstat.st_size + appended_zeroes,
                                prot, ptype, fh );

        assert(result);
        if ( buffer_len )
        {
            *buffer_len = (I64) fdstat.st_size + (I64) appended_zeroes;
        }

        pal_closefile(fh);
    }
    return result;
}



int
pal_munmap( void* addr, size_t size )
{
    int result = 0; // 0 = success
    result = munmap(addr, size);
    return result;
}


int
pal_init( void )
{
    int result = false;

    { // Page Size retrieve
        signal(SIGSEGV, sigsegv_linux_callback);   // install our handler
    }

    return result;
}


int
pal_createdir(char *path )
{
    return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}


bool
pal_get_file_last_access_info(char *filepath, FileLastAccessInfo *out)
{
    bool success = true;
    struct stat statbuf;
    int s = stat(filepath, & statbuf);
    if (s != 0)
    {
        success = false;
        memclr(out, sizeof(*out));
    }
    else
    {
        out->last_access.tv_sec = statbuf.st_atim.tv_sec;
        out->last_modification.tv_sec = statbuf.st_mtim.tv_sec;
    }

    return success;
}


int
pal_filetime_diff(FileTime *result,
                  FileTime *x,
                  FileTime *y)
{
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_nsec < y->tv_nsec)
    {
        time_t nsec = (y->tv_nsec - x->tv_nsec) / 1000000000ll + 1;
        y->tv_nsec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_nsec - y->tv_nsec > 1000000)
    {
        time_t nsec = (x->tv_nsec - y->tv_nsec) / 1000000000ll;
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
pal_filetime_cmp(FileTime *ft1,
                 FileTime *ft2)
{
    FileTime diff;

    int result = 0;
    {
        int is_negative = pal_filetime_diff (& diff, ft1, ft2);
        result = is_negative ? -1 : 1;
        if (diff.tv_sec == 0 && diff.tv_nsec == 0)
        {
            result = 0;
        }
    }

    return result;
}





I64
pal_readfile(FileHandle file, void *buf, I64 size_to_read)
{
    assert(size_to_read >= 0);
    I64 number_of_bytes_read = read(file, buf, (size_t) size_to_read);
#if __DEBUG
    if (number_of_bytes_read < 0)
    {
        // Place a breakpoint here for convenience to see the error message if any
        char *err_message = strerror(errno);
        int breakme;
        (void) err_message;
        (void) breakme;
    }
#endif
    return number_of_bytes_read;
}


I64
pal_writefile(FileHandle file, void *buf, I64 size_to_write)
{
    assert(size_to_write >= 0);
    I64 number_of_bytes_written = write(file, buf, (size_t) size_to_write);
#if __DEBUG
    if (number_of_bytes_written < 0)
    {
        // Place a breakpoint here for convenience to see the error message if any
        char *err_message = strerror(errno);
        int breakme;
        (void) err_message;
        (void) breakme;
    }
#endif
    return number_of_bytes_written;
}



ProcHandle
pal_spawnproc_sync ( char *command, int * exit_status )
{
    pid_t id = fork();
    if (id == - 1)
    {
        // error on fork
        return Invalid_ProcHandle;
    }
    else if ( id == 0 )
    {
        // child process
        if (execl("/bin/sh", "sh", "-c", command, (char *) 0) )
        {
            assert(0);
            abort();
        }
    }
    else
    {
        // parent process
        // wait for child to complete
        waitpid(-1, exit_status, 0);
    }
    return id;
}




ProcHandle
pal_spawnproc_async ( char * command )
{
    pid_t id = fork ();
    if ( id == - 1)
    {
        // error on fork
        return Invalid_ProcHandle;
    }
    else if ( id == 0 )
    {
        // child process
        if ( execl("/bin/sh", "sh", "-c", command, (char *) 0) )
        {
            assert(0);
            abort();
        }
    }
    else
    {
        // parent process
    }
    return id;
}


ProcHandle
pal_spawnproc_async_piped ( char *command,
                            FileHandle *inpipe, FileHandle *outpipe)
{
    FileHandle outpipes[2] = {0};
    FileHandle inpipes[2] = {0};
    int piperes = pipe(outpipes);
    UNUSED(piperes);
    assert(piperes == 0);
    piperes = pipe(inpipes);
    assert(piperes == 0 );
    pid_t pid = fork ();
    if ( pid == -1)
    {
        // error on fork
        assert_msg(0, "Failed to fork the requested proc");
        return Invalid_ProcHandle;
    }
    else if ( pid == 0 )
    {
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
            assert(0);
            abort();
        }
    }
    else
    {
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
}


void
pal_syncproc ( ProcHandle proc, int *status )
{
    assert(proc != Invalid_ProcHandle);
    int local_status;
    waitpid ( proc, (status != NULL) ? status : &local_status, 0 );
}



FileHandle
pal_create_notify_instance(void)
{
    return inotify_init1(IN_NONBLOCK);
}

static inline U32
notify_event_flags__convert(enum notify_event_flags flags)
{
    U32 result = 0;
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
}


static inline enum notify_event_flags
linux_inotify_event_mask__convert(U32 mask)
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


FileHandle
pal_notify_start_watch_file(FileHandle notify_instance_fh,
                            const char *file_path,
                            enum notify_event_flags flags)
{
    assert(notify_instance_fh != Invalid_FileHandle);
    U32 mask = notify_event_flags__convert(flags);
    FileHandle result = inotify_add_watch(notify_instance_fh, file_path, mask);
    if (result != Invalid_FileHandle)
    {
        int r = fcntl(result, F_SETFL, fcntl(result, F_GETFL, 0) | O_NONBLOCK);
        if (r == -1)
        {
            fprintf(stderr, "Failed to set NONBLOCK mode for `inotify` component\n");
        }
    }
    return result;
}

void
pal_notify_end_watch_file(FileHandle notify_instance_fh,
                          FileHandle watch_descriptor)
{

    assert(notify_instance_fh && notify_instance_fh != Invalid_FileHandle);
    assert(watch_descriptor && watch_descriptor != Invalid_FileHandle);
    inotify_rm_watch(notify_instance_fh, watch_descriptor);
}



bool
pal_read_notify_event(FileHandle notify_instance_fh,
                      struct notify_event *output)
{
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
}



DllHandle
pal_dll_open(char *path)
{
    DllHandle result = 0;
    result = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    return result;
}

void
pal_dll_close(DllHandle handle)
{
    assert(handle != Invalid_DllHandle);
    int dlres = dlclose(handle);
    (void) dlres;
    assert(dlres == 0);
}

void *
pal_get_proc_addr(DllHandle handle,
                  const char *symbol_name)
{
    void *result = NULL;
    assert(handle != Invalid_DllHandle);
    result = dlsym(handle, symbol_name);
    return result;
}
