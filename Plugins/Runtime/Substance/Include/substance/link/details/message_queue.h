/** @file message_queue.h
    @brief Contains all messages to be processed both inbound and outbound
    @author Galen Helfter - Allegorithmic
    @date 20181103
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_MESSAGE_QUEUE_H
#define _SUBSTANCE_LINK_MESSAGE_QUEUE_H

#include <substance/link/details/message.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Perform any initialization operations for setup of the message queues */
unsigned int link_init_message_queue_subsystem(void);

/* Performs shutdown operations on the inbound and outbound queues */
unsigned int link_shutdown_message_queue_subsystem(void);

/* Emplaces the given message at the end of the end of the inbound queue */
void link_enqueue_inbound_message(link_message_t *message);

/* Acquires the front inbound message off the inbound message queue */
link_message_t* link_acquire_inbound_message(void);

/* Emplaces the given message at the end of the end of the outbound queue */
void link_enqueue_outbound_message(link_message_t *message);

/* Acquires the front outbound message off the outbound message queue */
link_message_t* link_acquire_outbound_message(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_MESSAGE_QUEUE_H */
