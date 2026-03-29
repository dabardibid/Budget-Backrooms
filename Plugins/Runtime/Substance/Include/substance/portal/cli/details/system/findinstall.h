/** @file findinstall.h
    @brief Contains utility functions for finding an install
    @author Galen Helfter - Adobe
    @date 20190821
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_PORTAL_CLI_DETAILS_SYSTEM_FINDINSTALL_H
#define _SUBSTANCE_PORTAL_CLI_DETAILS_SYSTEM_FINDINSTALL_H

#include <substance/link/details/system/pathtype.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/** @brief Locate a program, if it currently exists
    @param location System-specific location. This is the path on Unix, and
               the expected registry key on Windows.
    @param secondary Secondary system-specific location. This is unused on
               Unix, and is the subkey on Windows.
    @param program_name The expected string name of the program executable
    @return The verified path to the executable, or NULL on failure.
*/
link_pathchar_t* portal_cli_find_install(const link_pathchar_t *location,
                                         const link_pathchar_t *secondary,
                                         const link_pathchar_t *program_name);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_PORTAL_CLI_DETAILS_SYSTEM_FINDINSTALL_H */
