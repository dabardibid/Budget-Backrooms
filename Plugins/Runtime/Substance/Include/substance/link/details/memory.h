/** @file memory.h
    @brief Contains memory operations, such as for shared memory, etc.
    @author Galen Helfter - Allegorithmic
    @date 20181030
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_DETAILS_MEMORY_H
#define _SUBSTANCE_LINK_DETAILS_MEMORY_H

#include <stddef.h>

#include <substance/link/common.h>
#include <substance/link/types.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

typedef struct _link_shared_mem
{
    void *handle; /* Always call read and use this handle */

    size_t size;  /* Size of the shared memory handle */

#if defined(SUBSTANCE_LINK_WIN32)
    void *file_map; /* Only used on Windows - stores mapped file handle */
    wchar_t *path;  /* Only used on Windows - stores the path to the memory */
#elif defined(SUBSTANCE_LINK_POSIX)
    char *path;    /* Only used on POSIX - stores the path to the memory */
    int fd;        /* Only used on POSIX - file descriptor for open memory */
#endif
} link_shared_mem_t;

/* Resets the allocators to the system default */
unsigned int link_clear_allocators(void);

/* Sets the internal memory allocator to the given function pointer. An
 * appropriate deallocator should also be set with a link_set_deallocator
 * call. */
void link_set_allocator(substance_link_memory_allocate_fp allocator);

/* Sets the internal deallocator or free function. This should be set to
 * match the memory allocator function. */
void link_set_deallocator(substance_link_memory_free_fp deallocator);

/* Standard allocator for system memory, allocating memory with the set
 * internal allocator. */
void* link_allocate(size_t size);

/* Array allocator function, operating similarly to the link_allocate
 * function. */
void* link_array_allocate(size_t num, size_t size);

/* Deallocates the given memory, using the internal deallocator. This should
 * have been allocated using the link_allocate function. */
void link_free(void *ptr);

/* Memory handling for shared memory objects */
/* Acquisition expects that the size member has been set in the shared
 * memory object */
void link_acquire_shared_memory(link_shared_mem_t *mem);

/* Frees the given shared memory object. Will set all pointers to NULL and
 * clear the size. */
void link_free_shared_memory(link_shared_mem_t *mem);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_DETAILS_MEMORY_H */
