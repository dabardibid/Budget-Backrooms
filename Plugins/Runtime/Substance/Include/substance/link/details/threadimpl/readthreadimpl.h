/** @file readthreadimpl.h
    @brief Contains implementation functions for parts of the read threads
    @author Galen Helfter - Adobe
    @date 20190729
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_DETAILS_THREADIMPL_READTHREADIMPL_H
#define _SUBSTANCE_LINK_DETAILS_THREADIMPL_READTHREADIMPL_H

#include <substance/link/details/thread.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Function to signal main once the read thread has acquired its lock */
typedef void (*link_read_signal_main_fp)();

struct _link_read_thread;

unsigned int link_read_thread_handle_open(unsigned int context);

unsigned int link_read_thread_handle_connected(unsigned int context);

unsigned int link_read_thread_handle_context(unsigned int context);

unsigned int link_read_thread_await_condition(struct _link_read_thread *thread,
                                              link_cond_t *condition,
                                              link_mutex_t *lock,
                                              unsigned int *shutdown_flag,
                                              link_read_signal_main_fp notify);

unsigned int link_read_thread_handle_poll(struct _link_read_thread *thread);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_DETAILS_THREADIMPL_READTHREADIMPL_H */
