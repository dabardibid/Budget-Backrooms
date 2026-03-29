/** @file types.h
    @brief Contains type definitions needed for the external api
    @author Galen Helfter - Allegorithmic
    @date 20181121
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_TYPES_H
#define _SUBSTANCE_LINK_TYPES_H

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

typedef struct _substance_link_uuid
{
    uint32_t elements[4u];
} substance_link_uuid_t;

typedef void* (*substance_link_memory_allocate_fp)(size_t size);
typedef void (*substance_link_memory_free_fp)(void *ptr);

typedef void (*substance_link_trampoline_fp)(unsigned int context,
                                             const substance_link_uuid_t *type,
                                             const char* message);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_TYPES_H */
