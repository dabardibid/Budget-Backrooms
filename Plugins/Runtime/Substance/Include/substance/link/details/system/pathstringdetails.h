/** @file pathstringdetails.h
    @brief Contains various path string operation defines
    @author Galen Helfter - Adobe
    @date 20190723
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_DETAILS_SYSTEM_PATHSTRINGDETAILS_H
#define _SUBSTANCE_LINK_DETAILS_SYSTEM_PATHSTRINGDETAILS_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <substance/link/common.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

#if defined(SUBSTANCE_LINK_POSIX)
#define LINK_PATH_DELIM '/'
#define LINK_PATH_SNPRINTF snprintf
#define LINK_PATH_STRLEN strlen
#define LINK_PATH_FOPEN fopen
#define LINK_PATH_STRCMP strcmp
#define LINK_PATH_ATOI atoi
#define LINK_PATH_STRRCHR strrchr
#elif defined(SUBSTANCE_LINK_WIN32)
#define LINK_PATH_DELIM '\\'
#define LINK_PATH_SNPRINTF _snwprintf
#define LINK_PATH_STRLEN wcslen
#define LINK_PATH_FOPEN _wfopen
#define LINK_PATH_STRCMP wcscmp
#define LINK_PATH_ATOI _wtoi
#define LINK_PATH_STRRCHR wcsrchr
#else
#error "Currently only Windows and Posix string utilities are supported"
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_DETAILS_SYSTEM_PATHSTRINGDETAILS_H */
