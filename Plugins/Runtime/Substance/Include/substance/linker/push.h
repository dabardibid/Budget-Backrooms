/** @file push.h
	@brief Substance linker assembly/archive push structures
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance linker headers
	@date 20111018
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceLinkerPush and related structures.
	The SubstanceLinkerPush structure is used as parameter of the
	substanceLinkerHandlePushAssemblyEx function (handle.h).<BR>
*/


#ifndef _SUBSTANCE_LINKER_PUSH_H
#define _SUBSTANCE_LINKER_PUSH_H


/** Platform dependent definitions */
#include "platformdep.h"


/** @brief Buffer description structure: pair of pointer and size */
typedef struct
{
	const char* ptr;        /**< Pointer on data */
	size_t size;            /**< Size of the buffer pointed by 'ptr'. */

} SubstanceLinkerBuffer;


/** @brief Push assembly source type enumeration.
	Used in SubstanceLinkerPush structure. */
typedef enum
{
	Substance_Linker_Src_Buffer    = 0x0, /**< From buffer */
	Substance_Linker_Src_FilepathA = 0x1, /**< From ASCII filepath */
	Substance_Linker_Src_FilepathW = 0x2  /**< From UNICODE filepath (Windows-only) */

} SubstanceLinkerPushSrcType;


/** @brief Structure used as argument of substanceLinkerHandlePushAssemblyEx
	Describes the assembly (.SBSASM) or archive (.SBSAR) to link. */
typedef struct
{
	/** @brief Type of source: buffer or file (ASCII and UNICODE)
		The 'srcType' and 'src' members must be filled accordingly. */
	SubstanceLinkerPushSrcType srcType;

	/** @brief Assembly/archive filepath or pointer union
		Must be filled consistency with 'srcType' member. */
	union
	{
		/** @brief From buffer in memory.
			@warning If it references an assembly (SBSASM format), the buffer
				must be valid until calling substanceLinkerHandleLink.
			@note This buffer data is NOT copied.
			@note The size is only used if it references an archive (SBSAR
				format). */
		SubstanceLinkerBuffer buffer;

		/** @brief Filepath as ASCII null-terminated string */
		const char* filepathA;

		/** @brief Filepath as Unicode null-terminated string */
		const wchar_t* filepathW;

	} src;

	/** @brief User unique identifier used to tag this push operation
		Used to identified each push operation when building 'cache mapping',
		see substanceLinkerHandleGetCacheMapping. If not used, set to 0. */
	unsigned int uid;

} SubstanceLinkerPush;


#endif /* ifndef _SUBSTANCE_LINKER_PUSH_H */
