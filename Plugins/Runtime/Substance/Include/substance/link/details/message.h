/** @file message.h
    @brief Contains the definitions for working with messages
    @author Galen Helfter - Allegorithmic
    @date 20181103
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_MESSAGE_H
#define _SUBSTANCE_LINK_MESSAGE_H

#include <stdint.h>
#include <stddef.h>

#include <substance/link/types.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

typedef struct _link_message_header_r1 link_message_header_t;

typedef struct _link_message
{
    link_message_header_t *header; /* Message header */
    char *message;                 /* Buffer containing the message data */
    unsigned int context;          /* Context identifier */
} link_message_t;

link_message_t* link_build_message(unsigned int context,
                                   const substance_link_uuid_t *type,
                                   const char *message);
                                     

/* Clears a message structure, freeing any internal memory that it holds and
 * zeroing out any values. */
void link_clear_message(link_message_t *message);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_MESSAGE_H */
