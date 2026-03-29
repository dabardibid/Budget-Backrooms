/** @file locked_queue.h
    @brief Contains the data structure for a generic locked queue
    @author Galen Helfter - Allegorithmic
    @date 20190104
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_DETAILS_LOCKED_QUEUE_H
#define _SUBSTANCE_LINK_DETAILS_LOCKED_QUEUE_H

#include <substance/link/details/thread.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

typedef void (*link_locked_queue_node_handler_fp)(void *node);

typedef struct _link_locked_queue_node
{
    struct _link_locked_queue_node *next;
    void *contents;
} link_locked_queue_node_t;

typedef struct _link_locked_queue
{
    struct _link_locked_queue_node *front;
    struct _link_locked_queue_node *end;

    /* Allow a customized destructor for all nodes */
    link_locked_queue_node_handler_fp handler;
    link_mutex_t lock;
} link_locked_queue_t;

/* Initializes the mutex lock and the pointers in the queue. Undefined
 * behavior if the queue has already been initialized. */
void link_locked_queue_init(link_locked_queue_t *queue,
                            link_locked_queue_node_handler_fp handler);

/* Clears the given queue, removing all nodes and passing them to the handler
 * function. */
void link_locked_queue_clear(link_locked_queue_t *queue);

/* Enqueue the given node onto the queue, will perform a lock on the queue
 * to ensure proper addition. */
void link_locked_enqueue(link_locked_queue_t *queue,
                         link_locked_queue_node_t *node);

/* Dequeue the first node off the queue. Will perform a lock on the queue to
 * ensure proper removal. */
link_locked_queue_node_t* link_locked_dequeue(link_locked_queue_t *queue);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_DETAILS_LOCKED_QUEUE_H */
