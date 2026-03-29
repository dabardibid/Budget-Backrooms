/** @file state.h
    @brief Contains internal module state
    @author Galen Helfter - Allegorithmic
    @date 20181214
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_DETAILS_STATE_H
#define _SUBSTANCE_LINK_DETAILS_STATE_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

typedef struct _link_state
{
    /* State machine for the current system */
    uint32_t state;
} link_state_t;

extern link_state_t link_module_state;

/* Valid values for the state machine driving the library. Ensuring
 * proper state with atomic calls will make it so that external API
 * calls are much safer in a threaded context. */
enum SubstanceLinkState
{
    /* Standard states */

    /* Module has not been initialized */
    SUBSTANCE_LINK_STATE_SHUTDOWN = 0x0u,
    /* Module has started initialization */
    SUBSTANCE_LINK_STATE_INIT_STARTED = 0x1u,
    /* Module has finished initialization */
    SUBSTANCE_LINK_STATE_INIT_FINISHED = 0x2u,
    /* Shutdown is currently in progress */
    SUBSTANCE_LINK_STATE_SHUTDOWN_STARTED = 0x3u,
    /* Currently altering the memory allocators */
    SUBSTANCE_LINK_STATE_ALLOCATOR_STARTED = 0x4u,

    /* Error states */

    /* Internal error - undefined behavior */
    SUBSTANCE_LINK_STATE_INTERNAL_ERROR = 0x128u
};

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_DETAILS_STATE_H */
