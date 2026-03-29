/** @file callbacks.h
	@brief Substance callbacks signatures and related defines
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance linker headers
	@date 20100406
	@copyright Allegorithmic. All rights reserved.

	Defines callback function signatures used by handle and context structures.
*/

#ifndef _SUBSTANCE_LINKER_CALLBACKS_H
#define _SUBSTANCE_LINKER_CALLBACKS_H

/** Handle structure and related functions definitions pre-declaration */
struct SubstanceLinkerHandle_;

/** Platform dependent definitions */
#include "platformdep.h"


/** @brief Callback types enumeration

	This enumeration is used to name callbacks in the Substance linker context
	structure.*/
typedef enum
{
	Substance_Linker_Callback_Dependency,
	Substance_Linker_Callback_Error,
	Substance_Linker_Callback_ArchiveXml,
	Substance_Linker_Callback_ArchiveAsm,
	Substance_Linker_Callback_UIDCollision,
	Substance_Linker_Callback_ArchiveExtra,
	/* etc. */

	Substance_Linker_Callback_Internal_Count

} SubstanceLinkerCallbackEnum;


/** @brief UID Collision type

	Used by the Substance_Linker_Callback_UIDCollision callback. */
typedef enum
{
	Substance_Linker_UIDCollision_Output  = 0x0, /**< Outputs UIDs collision */
	Substance_Linker_UIDCollision_Input  = 0x1   /**< Inputs UIDs collision */

} SubstanceLinkerUIDCollisionType;


/* Begin of the EXTERNC block (if necessary) */
#ifdef __cplusplus
SUBSTANCE_EXTERNC {
#endif /* ifdef __cplusplus */


/** @typedef void SubstanceLinkerCallbackDependency
	@brief Type of the dependency required callback function

	@code void (*)(
		SubstanceLinkerHandle* handle,
		const wchar_t* assemblyFilepath,
		unsigned int assemblyUID) @endcode
		
	This callback is called each time the linker requires an new assembly used 
	by the currently processed assembly. One of the 
	substanceLinkerHandlePushAssembly... function must be called before 
	returning from this callback, in order to provide the required assembly.

	The 'handle' parameter is the Substance handle that calls this callback
	function.<BR>
	The 'assemblyFilepath' parameter contains the filepath of the required
	assembly. This is the cooking time filepath. It's an UNICODE null-terminated
	string.<BR>
	The 'assemblyUID' parameter contains the unique identifier of the required
	assembly.*/
typedef void (SUBSTANCE_CALLBACK *SubstanceLinkerCallbackDependency)(
	struct SubstanceLinkerHandle_ *handle,
	const wchar_t* assemblyFilepath,
	unsigned int assemblyUID);

	
/** @typedef void SubstanceLinkerCallbackError
	@brief Type of the error message callback function

	@code void (*)(
		SubstanceLinkerHandle* handle,
		unsigned int severity,
		const char* msg) @endcode

	The 'handle' parameter is the Substance handle that calls this callback
	func.<BR>
	The 'severity' parameter describe the error severity 
	(0: warning, 1: error, 2: critical error).<BR>
	The 'msg' parameter contains the error message string (ASCII format). */
typedef void (SUBSTANCE_CALLBACK *SubstanceLinkerCallbackError)(
	struct SubstanceLinkerHandle_ *handle,
	unsigned int severity,
	const char* msg);
	
	
/** @typedef void SubstanceLinkerCallbackArchiveXml
	@brief Type of the archive Input/Output bindings XML callback function

	@code void (*)(
		SubstanceLinkerHandle* handle,
		const unsigned short* basename,
		const char* xmlContent) @endcode
		
	This callback is called each time the linker found a Input/Output bindings
	description file (XML formatted) into a Substance Archive (SBSAR file)
	opened by a substanceLinkerHandlePushAssembly... function call.

	The 'handle' parameter is the Substance handle that calls this callback
	function.<BR>
	The 'basename' parameter contains the basename of the XML file into the
	archive (same basename than corresponding assembly). It's an UTF16
	null-terminated string. No more valid after the callback return.<BR>
	The 'xmlContent' parameter is a pointer on the content of the Input/Output
	bindings description file found into the archive. It's an UTF8
	null-terminated string. No more valid after the callback return.*/
typedef void (SUBSTANCE_CALLBACK *SubstanceLinkerCallbackArchiveXml)(
	struct SubstanceLinkerHandle_ *handle,
	const unsigned short* basename,
	const char* xmlContent);


/** @typedef void SubstanceLinkerCallbackArchiveAsm
	@brief Type of the archive Assembly Data callback function

	@code void (*)(
		SubstanceLinkerHandle* handle,
		const unsigned short* basename,
		const char* xmlContent) @endcode

	This callback is called each time the linker found a Substance Assembly
	file into a Substance Archive (SBSAR file)
	opened by a substanceLinkerHandlePushAssembly... function call.

	This callback is used to extract Assembly content from SBSAR file: it's
	an advanced seldom usage.

	The 'handle' parameter is the Substance handle that calls this callback
	function.<BR>
	The 'basename' parameter contains the basename of the Assembly file into the
	archive. It's an UTF16 null-terminated string. No more valid after the
	callback return.<BR>
	The 'asmContent' parameter is a pointer on the content of the assembly. No
	more valid after the callback return.<BR>
	The 'asmSize' parameter is the assembly content size. */
typedef void (SUBSTANCE_CALLBACK *SubstanceLinkerCallbackArchiveAsm)(
	struct SubstanceLinkerHandle_ *handle,
	const unsigned short* basename,
	const char* asmContent,
	size_t asmSize);


/** @typedef void SubstanceLinkerCallbackArchiveExtra
	@brief Type of the archive extra content callback function

	@code void (*)(
		SubstanceLinkerHandle* handle,
		const unsigned short* basename,
		const void* extraContent,
		size_t extraSize) @endcode

	This callback is called each time the linker found a unidentified file in
	the Substance Archive (SBSAR file) opened by a
	substanceLinkerHandlePushAssembly... function call.

	The 'handle' parameter is the Substance handle that calls this callback
	function.<BR>
	The 'basename' parameter contains the basename of the extra file into
	the archive. It's an UTF16 null-terminated string. It is longer valid after
	the callback returns.<BR>
	The 'extraContent' parameter is a pointer to the file contents.
	It is an encoded buffer if the file was encoded. It is no longer valid
	after the callback has returned. It is stripped of a null terminator.
	The 'extraSize' parameter is the file buffer size. */
typedef void(SUBSTANCE_CALLBACK *SubstanceLinkerCallbackArchiveExtra)(
	struct SubstanceLinkerHandle_ *handle,
	const unsigned short* basename,
	const void* extraContent,
	size_t extraSize);


/** @typedef void SubstanceLinkerCallbackUIDCollision
	@brief Type of the UID collision notification callback function

	@code void (*)(
		SubstanceLinkerHandle* handle,
		SubstanceLinkerUIDCollisionType collisionType,
		unsigned int previousUid,
		unsigned int newUid) @endcode

	This callback is called each time the linker found a collision in inputs and
	outputs UIDs, during a substanceLinkerHandlePushAssembly... function call.
	
	This callback is used to notify that an UID contained in a Substance 
	Assembly is already used, and provides a new UID as a replacement.

	The 'handle' parameter is the Substance handle that calls this callback
	function.<BR>
	The 'collisionType' parameter indicates if it's an input or output UID 
	collision.<BR>
	The 'previousUid' parameter is the UID that collides.<BR>
	The 'newUid' parameter is the UID used as a replacement. */
typedef void (SUBSTANCE_CALLBACK *SubstanceLinkerCallbackUIDCollision)(
	struct SubstanceLinkerHandle_ *handle,
	SubstanceLinkerUIDCollisionType collisionType,
	unsigned int previousUid,
	unsigned int newUid);


#ifdef __cplusplus
}
/* End of the EXTERNC block (if necessary) */
#endif /* ifdef __cplusplus */

#endif /* ifndef _SUBSTANCE_LINKER_CALLBACKS_H */
