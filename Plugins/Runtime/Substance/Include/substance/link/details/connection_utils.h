/** @file connection_utils.h
    @brief Contains utilities for all platform connection implementations
    @author Galen Helfter - Allegorithmic
    @date 20181217
    @copyright Allegorithmic. All rights reserved.
 */

#ifndef _SUBSTANCE_LINK_CONNECTION_UTILS_H
#define _SUBSTANCE_LINK_CONNECTION_UTILS_H

#include <substance/link/common.h>
#include <substance/link/details/message.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

#if defined(SUBSTANCE_LINK_POSIX)
#include <poll.h>
#elif defined(SUBSTANCE_LINK_WIN32)
#include <winsock2.h>
#endif

/* Enum specifying return codes for a select or polling call */
enum SubstanceLinkPollError
{
    /* Success */
    SUBSTANCE_LINK_POLL_ESUCCESS = 0u,
    /* Bad descriptors */
    SUBSTANCE_LINK_POLL_EBADF    = 1u,
    /* File descriptor structures were not part of the address space */
    SUBSTANCE_LINK_POLL_EFAULT   = 2u,
    /* Interrupted before any processing */
    SUBSTANCE_LINK_POLL_EINTR    = 3u,
    /* The network system has failed */
    SUBSTANCE_LINK_POLL_ENETDOWN = 4u,
    /* No memory to allocate internal structures */
    SUBSTANCE_LINK_POLL_ENOMEM   = 5u,
    /* Invalid timeout value */
    SUBSTANCE_LINK_POLL_EINVAL   = 6u,
    /* Unknown error */
    SUBSTANCE_LINK_POLL_EUNKNOWN = 7u,

    /* Timed out of the polling call */
    SUBSTANCE_LINK_POLL_TIMEOUT  = 1200u
};

enum SubstanceLinkPollEvent
{
    /* Inbound data can be retrieved without blocking */
    SUBSTANCE_LINK_POLLIN  = POLLIN,
    /* An error has occurred with the given file descriptor */
    SUBSTANCE_LINK_POLLERR = POLLERR,

    /* Only supported using a call to poll instead of select */

    /* The other side has closed the socket */
    SUBSTANCE_LINK_POLLHUP = POLLHUP
};

/* Define the poll type as a pollfd, so that the structure may be used
 * efficiently in poll calls. May need to add a manual define for the
 * structure as well if we come across a system that does not define
 * the pollfd struct. */
typedef struct pollfd link_poll_t;

/* Blocks on each context, waiting for events to occur. Will return either
 * one of the error codes in the SubstanceLinkPollError enum or will return
 * SUBSTANCE_LINK_POLL_TIMEOUT, indicating that the poll timed out. A return
 * of SUBSTANCE_LINK_ESUCCESS indicates that the call succeeded, placing the
 * proper events into the poll structure. */
unsigned int link_poll_contexts(link_poll_t *contexts,
                                unsigned int context_count);

/* Convert a Substance Link header from host byte ordering to network
 * byte ordering */
void link_htonheader(link_message_header_t *target,
                     const link_message_header_t *original);

/* Convert a Substance Link header from network byte ordering to host
 * byte ordering */
void link_ntohheader(link_message_header_t *target,
                     const link_message_header_t *original);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_CONNECTION_UTILS_H */
