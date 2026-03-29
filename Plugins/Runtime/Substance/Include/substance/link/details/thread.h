/** @file thread.h
    @brief Contains threading declarations agnostic of platform
    @author Galen Helfter - Allegorithmic
    @date 20181121
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_THREAD_H
#define _SUBSTANCE_LINK_THREAD_H

#include <substance/link/common.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Currently we drive the threading through raw typedefs. This could be
 * changed to forward declare a structure as an opaque type and work
 * through that instead, but that would complicate it some and change the
 * threading API. */
#if defined(SUBSTANCE_LINK_POSIX)
#include <pthread.h>
typedef pthread_mutex_t link_mutex_t;
typedef pthread_cond_t link_cond_t;
typedef pthread_t link_thread_t;
typedef void* link_thread_return_t;
#elif defined(SUBSTANCE_LINK_WIN32)
/* Note: This is a very heavy header. Should look at trying to
 * manually define it. */
#include <Windows.h>
typedef SRWLOCK link_mutex_t;
typedef CONDITION_VARIABLE link_cond_t;
typedef HANDLE link_thread_t;
typedef unsigned long link_thread_return_t;
#endif

typedef link_thread_return_t (*link_thread_routine)(void*);

/* Creates a new mutex on the given platform, performing any initialization.
 * The mutex will be returned in an unlocked state. */
link_mutex_t link_mutex_create(void);

/* Destroys the given mutex. The mutex must be unlocked, as destroying a
 * locked mutex is undefined behavior. */
void link_mutex_destroy(link_mutex_t *mutex);

/* Performs a standard, blocking lock call on the given mutex */
void link_mutex_lock(link_mutex_t *mutex);

/* Unlocks the given mutex */
void link_mutex_unlock(link_mutex_t *mutex);

/* Creates a new thread executing the given routine */
link_thread_t link_thread_create(link_thread_routine routine, void *arg);

/* Waits for the specified thread to terminate */
void link_thread_join(link_thread_t *thread);

/* Destroys the given thread on the current platform */
void link_thread_destroy(link_thread_t *thread);

/* Creates a condition variable for the given platform */
void link_condition_create(link_cond_t *cond);

/* Destroys a valid condition variable */
void link_condition_destroy(link_cond_t *cond);

/* Given a lock and a condition variable, puts the current thread to sleep
 * until flagged again by a later event */
void link_condition_wait(link_cond_t *cond, link_mutex_t *lock);

/* Given a condition variable, signals a single consumer thread to wake up
 * and start processing */
void link_condition_signal(link_cond_t *cond);

/* Given a condition variable, signals all awaiting threads to wake up
 * and start processing */
void link_condition_broadcast(link_cond_t *cond);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_THREAD_H */
