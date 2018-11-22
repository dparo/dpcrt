#ifndef HGUARD_6cae59f8ded7434090c01c15fe03a866
#define HGUARD_6cae59f8ded7434090c01c15fe03a866

#include "types.h"
#include "utils.h"

#include

__BEGIN_DECLS

/* Simple, easy to use for quick testing
   stack allocator implementation based on
   `malloc`, `free`, `realloc` */
typedef struct AllocationChunk
{
    /* Size of the chunk that was allocated successfully.
       In case the allocation failed, this field _should_
       be set to 0, to avoid usage of uninitialized memory. */
    size_t  size_of_chunk;

    /* Pointer to the actual allocated block of memory.
       It may be `NULL` in case the allocation failed */
    void    *data;
} AllocationChunk;


typedef struct BufferU32
{
    U32   total_size;
    U32   usage_size;
    void *data;
} BufferU32;

typedef struct MemRefHandle
{
    U32 offset;
    U32 tag_metadata;
} MemRefHandle;



typedef struct SAPushContext
{
    B32 valid;
    U32 pushed_memory_size;
}  SAPushContext;

typedef struct IFACE_StackAllocator
{
    SAPushContext (*begin) (void *self);
    bool          (*push)  (void *self, SAPushContext *context, ptr_t data, U32 size);
    bool          (*end)   (void *self, SAPushContext *context);
    void          (*clear) (void *self);
} IFACE_StackAllocator;



/* @NOTE :: Recall that `mem_ref_t = 0` is reserved value
   for indicating allocation failure. Stack Allocators
   implementations may want to skip the first byte in
   order to easily reserve the `0` index of the buffer. */
typedef struct IFACE_StackAllocator
{
    /* Advance the stack pointer (not necessarly pushing zero data) */
    mem_ref_t   (*advance)            (void *self, U32 size);
    /* Push zero-initialized data of len `size`.
       Same as `advance` but it is _GUARANTEED_ to push zero initialized
       data when advancing the Stack Pointer. */
    mem_ref_t   (*push_null)          (void *self, U32 size);
    /* Pushes a struct/object or whatever piece of memory of bytes `size` */
    mem_ref_t   (*push_data)          (void *self, void *data, U32 size);
    mem_ref_t   (*push_str32)         (void *self, Str32 str);
    mem_ref_t   (*push_pstr32)        (void *self, PStr32 str);
    mem_ref_t   (*push_cstr)          (void *self, char *str);

    /* Returns the address of the object contained at position refered
       from the memory handle. Note that this can potentially be an
       **unsafe** operation. In particular you shouldn't store
       the pointer anywhere, and you should not keep the pointer
       between `push_xxx` boundaries, since the pointer is not guaranteed
       to remain valid. */
    void*       (*addr_of)            (void *self, MemRefHandle mhandle);
    /* Resets the stack pointer back to position `pos`.

       @NOTE :: This stack allocator interface does not provide a `pop` functionality
       that just simply pops the last allocation. In order to achieve
       something like this, since a a stack allocator does not
       contain **homogenous data** (remember this is not a stack data structure,
       it is a stack _ALLOCATOR_)
       a stack allocator would need to internally
       maintain another stack that keeps track of all the `mem_ref_t`
       position of the stack pointer. This makes the implementation more
       difficult and this functionality might never be used. So if you really want
       this `pop` functionality, you need to use 2 Stack Allocators:
         - 1 Allocator for pushing whatever **non-homogeneous** data you might want.
         - 1 other Allocator for actually pushing only references `mem_ref_t` (homogenous-data)*/
    void        (*reset_sp)           (void *self, mem_ref_t pos);

    /* Clear the buffer of the allocator (possibly freeing it (implementation defined)):
       Do not actually delete the allocator itself */
    void        (*clear)              (void *self);

#if 0
    /* TODO :: Do we need this ?? */

    /* Deallocates the buffer of the allocator _AND_ the allocator itself.
       After this call, calling any function from this interface
       is undefined behaviour */
    void        (*free)               (void *self);
    bool        (*fetch)              (void *self, mem_ref_t ref, void *output, U32 size);
#endif


    /* Advance the stack pointer to ensure alignment.
       This can be implemented by simply computing
       how much the stack pointer needs to advance and calling `advance` function pointer. */
    U32         (*ensure_alignment)   (void *self, U32 alignment);
    /* Returns the actual allocation buffer. */
    BufferU32   (*get_buffer)         (void *self);
    U32         (*get_usage_size)     (void *self);
    void        (*fit_buffer)         (void *self);
} IFACE_StackAllocator;


/* @NOTE :: A generic allocator is literally the most generic
   allocator that it is possible to achieve. Technically we
   could define the other allocators as special case of a generic
   allocator and implement those with a generic allocator. */
typedef struct IFACE_GenericAllocator
{
    /* Allocates a block of memory. The memory may contain the "garbage"
       and left to be uninitialized. The choice is left for the implementation. */
    AllocationChunk   (*malloc)   (void *self, size_t size);
    /* Same as malloc, but it _MUST_ guarantee to return a block of cleared memory. */
    AllocationChunk   (*calloc)   (void *self, size_t size);
    /* @NOTE :: Realloc _CAN_ actually return an allocation chunk that has more
       size than the requested one, this is left to be implementation defined.
       This is usefull for allocators that may want to return (for internal reasons)
       an allocation chunk that is PAGE_ALIGNED.

       @NOTE :: the realloc function wants as a parameter the `prev_chunk`
       (eg the pointer to the actual memory, and how much big the memory is).
       While for example the STANDARD C `realloc` function
       does not care about the size of the previous chunk, other implementations
       might do. For Example for a memory mapping based allocator
       the typical linux `mremap` call needs to know the size of the previous
       allocated memory.
    */
    AllocationChunk   (*realloc)  (void *self, AllocationChunk prev_chunk, size_t new_requested_size);
    void              (*free)     (void *self, AllocationChunk chunk_to_be_freed);
} IFACE_GenericAllocator;





typedef BufferU32 MallocBasedStackAllocator;




__END_DECLS

#endif /* HGUARD_6cae59f8ded7434090c01c15fe03a866 */
