/** @file connection.h
    @brief Defines the internal connection API
    @author Galen Helfter - Allegorithmic
    @date 20181012
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_CONNECTION_H
#define _SUBSTANCE_LINK_CONNECTION_H

#include <substance/link/details/context_struct.h>
#include <substance/link/details/message.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Used both internally and externally */
typedef unsigned int (*context_op_fp)(link_context_t *context);
typedef unsigned int (*message_op_fp)(link_context_t *context,
                                      link_message_t *message);

/* Opens a new connection given the context, of the appropriate
 * communication type. */
unsigned int link_open_connection(link_context_t *context);

/* Attempts to connect a context to another instance of link. Returns a
 * a standard error code, and sets the context to the connected state
 * on success. */
unsigned int link_bridge_connection(link_context_t *context);

/* With a given context, checks whether there's any data to be read */
/* unsigned int link_check_connection(link_context_t *context); */

unsigned int link_read_connection(link_context_t *context,
                                  link_message_t *message);

/* Given a context, reads a message out of the buffer. */
unsigned int link_write_connection(link_context_t *context,
                                   link_message_t *message);

/* Closes the context, disconnecting any sockets and closing any file
 * descriptors and files. */
unsigned int link_close_connection(link_context_t *context);

/* Accepts the given connection, assuming that it is a socket operation.
 * Returns a file descriptor instead of a standard error code, with -1
 * symbolizing an error in accepting the connection. The returned file
 * descriptor is not yet bound to a context, and must be registered
 * afterwards. */
int link_accept_connection(link_context_t *context);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_CONNECTION_H */
