/** @file invoke.h
    @brief Internal implementation of invoking the Launcher
    @author Galen Helfter - Adobe
    @date 20190821
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_PORTAL_CLI_DETAILS_INVOKE_H
#define _SUBSTANCE_PORTAL_CLI_DETAILS_INVOKE_H

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/** @brief Attempt to locate and execute the Launcher
    @return Errorcode repsenting success or failure
*/
unsigned int portal_cli_invoke_launcher(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_PORTAL_CLI_DETAILS_SYSTEM_FINDINSTALL_H */
