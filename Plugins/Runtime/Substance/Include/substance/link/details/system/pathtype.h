/** @file pathtype.h
    @brief Contains the system-specific path character type
    @author Galen Helfter - Adobe
    @date 20190719
    @copyright Adobe. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_DETAILS_SYSTEM_PATHTYPE_H
#define _SUBSTANCE_LINK_DETAILS_SYSTEM_PATHTYPE_H

#include <substance/link/common.h>

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

#if defined(SUBSTANCE_LINK_WIN32)
#include <wchar.h>
typedef wchar_t link_pathchar_t;
#else
typedef char link_pathchar_t;
#endif

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_DETAILS_SYSTEM_PATHTYPE_H */
