/** @file fileutils.h
    @brief Contains shared file and directory operations
    @author Galen Helfter - Adobe
    @date 20190719
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_DETAILS_SYSTEM_FILEUTILS_H
#define _SUBSTANCE_LINK_DETAILS_SYSTEM_FILEUTILS_H

#include <substance/link/common.h>
#include <substance/link/details/system/pathtype.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Macro used to expand out into a platform-specific literal string, such
 * as a wide string path for Windows, or a standard UTF-8 string path for
 * POSIX systems. */
#if defined(SUBSTANCE_LINK_WIN32)
#define LINK_PATH_STRING(value) L ## value
#elif defined(SUBSTANCE_LINK_POSIX)
#define LINK_PATH_STRING(value) value
#endif

/* Filter function type for directory operations. This function should
 * return SUBSTANCE_LINK_SUCCESS for any file matching the filter, and
 * another error code for any that do not. */
typedef unsigned int (*link_list_filter_fp)(link_pathchar_t *path,
                                            const void *data);

/* Acquires all of the elements in a directory. Takes the path of the
 * directory to inspect, along with a pointer to an array of null-terminated
 * strings. The pointer should be valid, but not point to a valid location,
 * as no cleanup is done. The array will have a NULL pointer sentinal at the
 * end of the list, denoting its end. An optional filter operation can be
 * passed as well to accept or deny files. This can be NULL for no filter. */
unsigned int link_list_directory(const link_pathchar_t *path,
                                 link_list_filter_fp filter_op,
                                 const void *data,
                                 link_pathchar_t ***elements);

/* Utility function to properly clean and delete the result of a list
 * directory operation. The elements array will be set to NULL as the
 * result of this. */
void link_clear_directory_list(link_pathchar_t ***elements);

/* Determines whether a given directory exists. Returns an errorcode
 * representing whether the directory exists. */
unsigned int link_is_directory(const link_pathchar_t *path);

/* Determines whether a Unix socket exists. Returns an errorcode
 * representing whether the file exists and is a unix socket. */
unsigned int link_is_unix_socket(const link_pathchar_t *path);

/* Determines whether a file exists. Returns an errorcode representing
 * whether the file exists and is a unix socket. */
unsigned int link_is_file(const link_pathchar_t *path);

/* Function to make a directory at a given location. This assumes that the
 * parent directories exist. Returns an errorcode representing success
 * or failure. */
unsigned int link_make_directory(const link_pathchar_t *path);

/* Removes a directory, assuming that it is an existing directory. Returns
 * an errorcode representing the success of removing the directory. */
unsigned int link_remove_directory(const link_pathchar_t *path);

/* Removes the file located at the given path. This will return an errorcode
 * representing the success of deleting the file. */
unsigned int link_remove_file(const link_pathchar_t *path);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_DETAILS_SYSTEM_FILEUTILS_H */
