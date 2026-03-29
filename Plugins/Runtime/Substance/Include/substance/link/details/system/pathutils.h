/** @file pathutils.h
    @brief Contains various path string utilities
    @author Galen Helfter - Adobe
    @date 20190722
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_DETAILS_SYSTEM_PATHUTILS_H
#define _SUBSTANCE_LINK_DETAILS_SYSTEM_PATHUTILS_H

#include <substance/link/common.h>
#include <substance/link/details/system/pathtype.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Joins the list of pieces into a final path. The result is stored in the
 * result pointer, which is a dynamically allocated string. This is the
 * responsibility of the caller to free, and can be freed using the functions
 * in substance/link/details/memory.h. The parts array is expected to be a
 * list of strings, with a NULL pointer sentinal marking its end. Returns an
 * errorcode representing success. */
unsigned int link_path_join(const link_pathchar_t **parts,
                            link_pathchar_t **result);

/* Duplicates a given path string, returning a duplicate of the proper
 * type for the platform. It is the responsibility of the caller to
 * free this using link_free */
link_pathchar_t* link_path_strdup(const link_pathchar_t *path);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_DETAILS_SYSTEM_PATHUTILS_H */
