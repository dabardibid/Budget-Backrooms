/** @file errorcodes.h
    @brief Contains the error codes for the link framework
    @author Galen Helfter - Allegorithmic
    @date 20181015
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_ERRORCODES_H
#define _SUBSTANCE_LINK_ERRORCODES_H

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

enum SubstanceLinkError
{
    SUBSTANCE_LINK_SUCCESS     = 0u,  /* Successful operation */
    SUBSTANCE_LINK_ERROR       = 1u,  /* General, nonspecific error */
    SUBSTANCE_LINK_BADALLOC    = 2u,  /* Failed memory allocation */
    SUBSTANCE_LINK_NOTLOADED   = 3u,  /* Dynamic module is not loaded */
    SUBSTANCE_LINK_CONN_FAIL   = 4u,  /* Connection fail, SIGPIPE */
    SUBSTANCE_LINK_SEND_FAIL   = 5u,  /* Write to a connection timed out */
    SUBSTANCE_LINK_UNSUPPORTED = 6u,  /* Platform unsupported for call */
    SUBSTANCE_LINK_INVALID     = 7u,  /* Invalid argument provided */
    SUBSTANCE_LINK_READ_FAIL   = 8u,  /* Failed read request */
    SUBSTANCE_LINK_OPEN_FAIL   = 9u,  /* Faied to open a connection */
    SUBSTANCE_LINK_ERROR_MAX   = 10u  /* Maximum current error codes */
};

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_ERRORCODES_H */
