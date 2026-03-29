/** @file runprocess.h
    @brief System utilities for running different processes
    @author Galen Helfter - Adobe
    @date 20190821
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_PORTAL_CLI_DETAILS_SYSTEM_RUNPROCESS_H
#define _SUBSTANCE_PORTAL_CLI_DETAILS_SYSTEM_RUNPROCESS_H

#include <substance/link/details/system/pathtype.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/** @brief Locate a program, if it currently exists
    @param location System-specific location. This is the path on Unix, and
               the expected registry key on Windows.
    @param argv Arguments to be passed to the executable
    @param env Environment setup to pass to the executable
    @return Errorcode representing success or failure
*/
unsigned int portal_cli_run_process(const link_pathchar_t *path,
                                    link_pathchar_t * const *argv,
                                    link_pathchar_t * const *env);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_PORTAL_CLI_DETAILS_SYSTEM_RUNPROCESS_H */
