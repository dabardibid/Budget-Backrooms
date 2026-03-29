/** @file cli.h
    @brief Defines the externally available API
    @author Galen Helfter - Adobe
    @date 20190821
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_PORTAL_CLI_CLI_H
#define _SUBSTANCE_PORTAL_CLI_CLI_H

#include <substance/link/common.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/** @brief Acquire the current version of the software
    @return Const C-string representing the current version
*/
SUBSTANCE_LINK_HEADER_EXPORT
const char* substance_portal_cli_get_version(void);

/** @brief Attempts to find and execute the Launcher
    @return Errorcode representing success or failure
*/
SUBSTANCE_LINK_HEADER_EXPORT
unsigned int substance_portal_cli_invoke(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_PORTAL_CLI_CLI_H */
