/** @file connection_details.h
    @brief Provides the specifics for connecting on the current platform
    @author Galen Helfter - Allegorithmic
    @date 20181126
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_CONNECTION_DETAILS_H
#define _SUBSTANCE_LINK_CONNECTION_DETAILS_H

#include <substance/link/details/context_struct.h>
#include <substance/link/details/message.h>

/* These function pointer typedefs map to the implementation-specific
 * functions laid out below. */
typedef unsigned int (*link_open_fp)(link_context_t *context);
typedef unsigned int (*link_connect_fp)(link_context_t *context);
typedef unsigned int (*link_read_fp)(link_context_t *context,
                                     link_message_t *message);
typedef unsigned int (*link_write_fp)(link_context_t *context,
                                      link_message_t *message);
typedef unsigned int (*link_close_fp)(link_context_t *context);

/* Accept functions return a file descriptor */
typedef int (*link_accept_fp)(link_context_t *context);

unsigned int link_open_tcp(link_context_t *context);
unsigned int link_connect_tcp(link_context_t *context);
unsigned int link_read_tcp(link_context_t *context, link_message_t *message);
unsigned int link_write_tcp(link_context_t *context, link_message_t *message);
unsigned int link_close_tcp(link_context_t *context);

int link_accept_tcp(link_context_t *context);

unsigned int link_open_unix(link_context_t *context);
unsigned int link_connect_unix(link_context_t *context);
unsigned int link_read_unix(link_context_t *context, link_message_t *message);
unsigned int link_write_unix(link_context_t *context, link_message_t *message);
unsigned int link_close_unix(link_context_t *context);

int link_accept_unix(link_context_t *context);

#endif /* _SUBSTANCE_LINK_CONNECTION_DETAILS_H */
