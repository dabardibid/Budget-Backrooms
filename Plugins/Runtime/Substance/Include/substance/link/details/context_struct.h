/** @file context_struct.h
    @brief Contains the structure specification for contexts
    @author Galen Helfter - Allegorithmic
    @date 20181111
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_CONTEXT_STRUCT_H
#define _SUBSTANCE_LINK_CONTEXT_STRUCT_H

#include <stdint.h>
#include <stddef.h>

/* Internal context structure - Even in the details, should not be used
 * except in places needing them (stored queue, opening communications,
 * etc.) */
typedef struct _link_context
{
    /* Bitfield element containing the following:
     * 8 bits - communication type (see SubstanceLinkCommunication enum
     * 3 bits - connection state (disconnected, open, connected, etc.)
     * 1 bit - whether a handshake has been performed */
     uint64_t configuration;
     size_t fd;   /* File descriptor/socket mapping to the connection */
     size_t port; /* Stores the port to bind to */
     void *connection_data;  /* Pointer to connection data, such as a string */
     char *application_name; /* String name of the connection */
     uint16_t identifier;
} link_context_t;

enum SubstanceLinkCommunication
{
    SUBSTANCE_LINK_COMM_TCP  = 0x01u,  /* TCP socket connection */
    SUBSTANCE_LINK_COMM_UNIX = 0x02u,  /* Unix local socket connection */
    SUBSTANCE_LINK_COMM_MAX  = 0x02u,
    SUBSTANCE_LINK_COMM_MASK = 0xffu   /* Mask for easy extraction */
};

enum SubstanceLinkConnState
{
    SUBSTANCE_LINK_CONN_CLOSED    = 0x000u,  /* Context is not connected */
    SUBSTANCE_LINK_CONN_OPEN      = 0x100u,  /* Context open for connections */
    SUBSTANCE_LINK_CONN_CONNECTED = 0x200u,  /* Context is connected */
    SUBSTANCE_LINK_CONN_SHUTDOWN  = 0x400u,  /* Context is ready for shutdown */
    SUBSTANCE_LINK_CONN_MASK      = 0x700u   /* Extraction mask */
};

enum SubstanceLinkHandshake
{
    SUBSTANCE_LINK_HANDSHAKE_NOT_SENT = 0x000u, /* No handshake sent */
    SUBSTANCE_LINK_HANDSHAKE_SENT     = 0x800u, /* Handshake has been sent */
    SUBSTANCE_LINK_HANDSHAKE_MASK     = 0x800u  /* Extraction mask */
};

#endif /* _SUBSTANCE_LINK_CONTEXT_STRUCT_H */
