/** @file common.h
    @brief Contains system defines and other common information
    @author Galen Helfter - Allegorithmic
    @date 20181012
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_COMMON_H
#define _SUBSTANCE_LINK_COMMON_H

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* Common platform defines */
#if defined(__ANDROID__)
#define SUBSTANCE_LINK_ANDROID 1
#define SUBSTANCE_LINK_POSIX   1
#elif defined(linux) || defined(__linux) || defined(__linux__)
#define SUBSTANCE_LINK_LINUX 1
#define SUBSTANCE_LINK_POSIX 1
#elif defined(__APPLE__)
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE
#define SUBSTANCE_LINK_IOS   1
#else
#define SUBSTANCE_LINK_MACOS 1 /* MacOS keeps a separate flag */
#endif
#define SUBSTANCE_LINK_POSIX 1
#elif defined(__unix__)
#define SUBSTANCE_LINK_UNIX  1 /* SUBSTANCE_LINK_UNIX for BSD/Solaris */
#define SUBSTANCE_LINK_POSIX 1
#elif defined(_WIN64)
#define SUBSTANCE_LINK_WIN64 1
#define SUBSTANCE_LINK_WIN32 1
#elif defined(_WIN32)
#define SUBSTANCE_LINK_WIN32 1
#endif

#if defined(SUBSTANCE_LINK_POSIX)
#define _POSIX_C_SOURCE 200809L
#ifndef   _DEFAULT_SOURCE
#define   _DEFAULT_SOURCE
#endif /* _DEFAULT_SOURCE */
#endif

#if defined(SUBSTANCE_LINK_MACOS) || defined(SUBSTANCE_LINK_IOS)
#ifndef   _DARWIN_C_SOURCE
#define   _DARWIN_C_SOURCE
#endif /* _DARWIN_C_SOURCE */
#endif

/* Dynamic library handle types */
#if defined(SUBSTANCE_LINK_POSIX)
#define SUBSTANCE_LINK_HANDLE void*
#elif defined(SUBSTANCE_LINK_WIN32)
#define SUBSTANCE_LINK_HANDLE HMODULE
#endif /* SUBSTANCE_LINK_POSIX */

/* External visibility types */
#if defined(SUBSTANCE_LINK_POSIX) /* This assumes GCC/Clang */
#define SUBSTANCE_LINK_EXPORT __attribute__((visibility("default")))
#define SUBSTANCE_LINK_HEADER_EXPORT
#elif defined(SUBSTANCE_LINK_WIN32)
#define SUBSTANCE_LINK_EXPORT __declspec(dllexport)
#define SUBSTANCE_LINK_HEADER_EXPORT __declspec(dllexport)
#endif /* SUBSTANCE_LINK_POSIX */

/* Define boolean mappings */
#define SUBSTANCE_LINK_FALSE 0
#define SUBSTANCE_LINK_TRUE  1

#define SUBSTANCE_LINK_UNUSED(x) (void)(x)

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_COMMON_H */
