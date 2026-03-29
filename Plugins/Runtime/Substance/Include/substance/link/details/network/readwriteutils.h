/** @file readwriteutils.h
    @brief Contains utility functions for reading from connections
    @author Galen Helfter - Adobe
    @date 20190626
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_DETAILS_NETWORK_READWRITEUTILS_H
#define _SUBSTANCE_LINK_DETAILS_NETWORK_READWRITEUTILS_H

#include <substance/link/common.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

struct _link_context;
struct _link_message;

#if defined(SUBSTANCE_LINK_POSIX)
#include <stddef.h>
#include <sys/types.h>
typedef ssize_t link_readwrite_size_t;
typedef size_t link_readwrite_buffersize_t;
#elif defined(SUBSTANCE_LINK_WIN32)
typedef int link_readwrite_size_t;
typedef int link_readwrite_buffersize_t;
#endif

typedef link_readwrite_size_t (*link_recv_fp)(int sock,
                                              void *buf,
                                              link_readwrite_buffersize_t len);

typedef link_readwrite_size_t (*link_send_fp)(int sock,
                                              const void *buf,
                                              link_readwrite_buffersize_t len);

unsigned int link_read_message_generic(struct _link_context *context,
                                       struct _link_message *message,
                                       link_recv_fp read_msg_fn);

unsigned int link_send_message_generic(const struct _link_context *context,
                                       const struct _link_message *message,
                                       link_send_fp send_msg_fn);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_DETAILS_NETWORK_READWRITEUTILS_H */
