/** @file searchpathunix.h
    @brief Searches the path for a matching program to invoke on Unix systems
    @author Galen Helfter - Adobe
    @date 20190823
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_PORTAL_CLI_DETAILS_SYSTEM_SEARCHPATHUNIX_H
#define _SUBSTANCE_PORTAL_CLI_DETAILS_SYSTEM_SEARCHPATHUNIX_H

#include <substance/link/details/system/pathtype.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/** @brief Filter function to only consider files starting with a name
    @param path File path to consider
    @param data String name to consider
    @return Errorcode representing whether the file is accepted or not
*/
unsigned int portal_cli_filter_file(link_pathchar_t *path, const void *data);

/** @brief Search for a program starting with the program name
    @param program Partial name of the program to search for.
    @param env_path Environment path string
    @return Path to the first executable to match. */
link_pathchar_t* portal_cli_search_program(const link_pathchar_t *program,
                                           const link_pathchar_t *env_path);

/** @brief Search for a program starting with the name in the system path
    @param program Partial name of the program to search for.
    @return Path to the first executable to match. */
link_pathchar_t* portal_cli_search_env_path(const link_pathchar_t *program);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_PORTAL_CLI_DETAILS_SYSTEM_SEARCHPATHUNIX_H */
