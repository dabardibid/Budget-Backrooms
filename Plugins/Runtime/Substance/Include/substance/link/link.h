/** @file link.h
    @brief Contains the function prototypes for the external API
    @author Galen Helfter - Allegorithmic
    @date 20181019
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_LINK_H
#define _SUBSTANCE_LINK_LINK_H

#include <substance/link/common.h>
#include <substance/link/types.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

struct substance_link_function_table
{
    const char* (*version)(void);
    unsigned int (*init)(const char*);
    unsigned int (*shutdown)(void);
    unsigned int (*set_allocators)(substance_link_memory_allocate_fp,
                                   substance_link_memory_free_fp);
    unsigned int (*write_message)(unsigned int, const substance_link_uuid_t*,
                                  const char*);
    unsigned int (*add_trampoline)(substance_link_trampoline_fp);
    unsigned int (*remove_trampoline)(substance_link_trampoline_fp);
    unsigned int (*open_tcp)(unsigned int, unsigned int*);
    unsigned int (*open_unix)(const char*, unsigned int*);
    unsigned int (*close_context)(unsigned int);
    unsigned int (*connect_tcp)(unsigned int, unsigned int*);
    unsigned int (*connect_unix)(const char*, unsigned int*);
    unsigned int (*broadcast_tcp)(void);
    unsigned int (*broadcast_unix)(void);
    unsigned int (*broadcast_default)(void);
    unsigned int (*open_default_tcp)(unsigned int*);
    unsigned int (*open_default_unix)(unsigned int*);
    unsigned int (*open_default)(unsigned int*);
};

/* Exported table of function pointers to all methods, to make dynamic loading
 * much easier. */
SUBSTANCE_LINK_HEADER_EXPORT
extern const struct substance_link_function_table substance_link_module_table;

/* Returns the string representation of the compiled semantic version */
SUBSTANCE_LINK_HEADER_EXPORT
const char* substance_link_version(void);

/* Perform the initial setup for the library */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_init(const char *application_name);

/* Perform final shutdown on the livelink connections */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_shutdown(void);

/* Pass in a memory allocator and deallocator to override the system memory
 * allocation */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_set_allocators(substance_link_memory_allocate_fp allocator,
                                           substance_link_memory_free_fp deallocator);

/* Write a message to the given context. Takes a unique ID type as a
 * parameter. */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_write_message(unsigned int context,
                                          const substance_link_uuid_t *type,
                                          const char *message);


/* Pass a trampoline function to receive all messages. This will be
 * provided by the language binding. A trampoline in this case is a function
 * designed allow entry into another programming language. Link will thus have
 * one point of entry into the interface language of the application, which
 * will then use the message type to determine the function or object in the
 * binding to call. */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_add_trampoline(substance_link_trampoline_fp
                                               trampoline);

/* Remove a trampoline from the internal trampoline list */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_remove_trampoline(substance_link_trampoline_fp
                                                trampoline);

/* Opens a new context for a TCP connection, taking the port to open on and a
 * pointer to return the context identifier through. */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_open_tcp(unsigned int port, unsigned int *context);

/* Opens a new context for a Unix connection, taking a pathname to bind to and
 * a pointer to return the context identifier through. */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_open_unix(const char *filepath,
                                      unsigned int *context);

/* Given a context identifier, closes the context and shuts down all
 * communication through it. The context identifier is invalid after
 * this operation. */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_close_context(unsigned int context);

/* Attempts to open a TCP connection to the given port. On success, it will
 * bind a new context and return that through the context pointer. */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_connect_tcp(unsigned int port,
                                        unsigned int *context);

/* Attempts to open a Unix domain socket connection to the given port. On
 * success, it will bind a new context and return that through the context
 * pointer. */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_connect_unix(const char *filepath,
                                         unsigned int *context);

/* Broadcasts connections using a tcp connection to attempt to connect to
 * all currently open instances of link, creating new contexts for each of
 * these. Returns an errorcode from errorcodes.h */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_broadcast_tcp(void);

/* Broadcasts connections using unix sockets, to connect to all open instances
 * of link with open Unix socket connections. This will create new contexts for
 * each connection established. This is only supported on Posix systems with
 * support for Unix domain sockets. Returns an errorcode from errorcodes.h */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_broadcast_unix(void);

/* Performs a broadcast connection, using the default connection type for the
 * platform. This will create new contexts for each connection. Returns an
 * errorcode from errorcodes.h */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_broadcast_default(void);

/* Opens a tcp socket, using a default port set up in a way that it can be
 * connected by another instance using a broadcast call. Returns an errorcode
 * representing success or failure. */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_open_default_tcp(unsigned int *context);

/* Opens a Unix socket, using a path set up for other instances to connect
 * using a broadcast call. Returns an errorcode representing success or
 * failure. */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_open_default_unix(unsigned int *context);

/* Opens a default connection, using the platform-specific default connection
 * type. This will be set up in a way that a default broadcast call from
 * another instance will establish a connection. Returns an errorcode
 * representing success or failure. */
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_link_open_default(unsigned int *context);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_LINK_H */
