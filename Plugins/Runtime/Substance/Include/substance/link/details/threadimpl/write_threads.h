/** @file write_threads.h
    @brief Contains the functionality for the write threads
    @author Galen Helfter - Adobe
    @date 20190625
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_DETAILS_THREADIMPL_WRITE_THREADS_H
#define _SUBSTANCE_LINK_DETAILS_THREADIMPL_WRITE_THREADS_H

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Initialize the write threads, handling their internal state. Returns an
 * errorcode representing success. */
unsigned int link_init_write_threads(void);

/* Perform a shutdown of the write threads. This will close them, clear their
 * resources and set them up to be reinitialized. */
unsigned int link_shutdown_write_threads(void);

/* Implementation of flagging the write threads, allowing it to be handled
 * internally. */
unsigned int link_flag_write_impl(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_DETAILS_THREADIMPL_WRITE_THREADS_H */
