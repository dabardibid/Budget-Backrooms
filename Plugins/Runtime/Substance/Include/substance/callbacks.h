/** @file callbacks.h
	@brief Substance callbacks signatures and related defines
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080108
	@copyright Allegorithmic. All rights reserved.

	Defines callback function signatures used by handle and context structures.
*/

#ifndef _SUBSTANCE_CALLBACKS_H
#define _SUBSTANCE_CALLBACKS_H

/** Handle structure and related functions definitions pre-declaration */
struct SubstanceHandle_;

/** Substance engine input texture structure pre-declaration */
struct SubstanceTextureInput_;

/** Platform dependent definitions */
#include "defines.h"


/** @brief Callback types enumeration

	This enumeration is used to name callbacks in the Substance context
	structure.*/
typedef enum
{
	Substance_Callback_Progress,
	Substance_Callback_MallocEx,
	Substance_Callback_OutputCompleted,
	Substance_Callback_OutOfMemory,
	Substance_Callback_JobCompleted,
	Substance_Callback_Malloc,
	Substance_Callback_Free,
	Substance_Callback_FreeEx,
	Substance_Callback_InPlaceOutput,
	Substance_Callback_InputImageLock,
	Substance_Callback_InputImageUnlock,
	Substance_Callback_CacheEvict,
	Substance_Callback_ProfileEvent,
	/* etc. */

	Substance_Callback_Internal_Count

} SubstanceCallbackEnum;


/** @brief MallocEx Callback flags enumeration

	This enumeration is used by the Substance_Callback_MallocEx callback.
	Specifies the platform memory type to allocate (exclusive flags). */
typedef enum
{
	Substance_MallocEx_MemTypeSystem  = 0x00,  /**< System memory (dflt.) */
	Substance_MallocEx_MemTypeWC      = 0x10,  /**< GPU-mapped WC memory  */
	Substance_MallocEx_MemTypeWB      = 0x20   /**< GPU-mapped WB memory  */

} SubstanceMallocExFlag;


/** @brief CacheEvict Callback flags enumeration

	This enumeration is used by the Substance_Callback_CacheEvict callback.
	Specifies if buffer is evicted, fetched and/or no more useful. The 'evict'
	and 'fetch' flags are exclusive. The 'remove' and 'fetch' flags can be set
	together: buffer is required by engine and can be discarded after. */
typedef enum
{
	Substance_CacheEvict_Evict   = 0x1,   /**< buffer is transfered to caller */
	Substance_CacheEvict_Fetch   = 0x2,   /**< buffer is required by engine */
	Substance_CacheEvict_Remove  = 0x4    /**< the buffer is no more required */

} SubstanceCacheEvictFlag;


/** @brief ProfileEvent type enumeration

	This enumeration is used by the Substance_Callback_ProfileEvent callback.
	Internal use only. */
typedef enum
{
	Substance_ProfileEvent_ComputeBegin,
	Substance_ProfileEvent_ComputeEnd

} SubstanceProfileEvent;


/* Begin of the EXTERNC block (if necessary) */
#ifdef __cplusplus
SUBSTANCE_EXTERNC {
#endif /* ifdef __cplusplus */


/** @typedef void SubstanceCallbackProgress
	@brief Type of the progress callback function

	@code void (*)(SubstanceHandle* handle,unsigned int progress,
		unsigned int progressMax) @endcode

	The 'handle' parameter is the Substance handle that calls this callback
	func.<BR>
	The 'progress' parameter varies between 0 and progressMax-1. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackProgress)(
	struct SubstanceHandle_ *handle,
	unsigned int progress,
	unsigned int progressMax);


/** @typedef void SubstanceCallbackOutputCompleted
	@brief Type of the 'result output texture completed' callback function

	@code void (*)(SubstanceHandle* handle,unsigned int outputIndex,
		size_t jobUserData) @endcode

	This callback is called every time an output texture has been rendered.
	
	The 'handle' parameter is the Substance handle that calls this callback
	func.<BR>
	The 'outputIndex' parameter is the index of the completed output.<BR>
	'jobUserData' is the user information of the job corresponding to this
	output. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackOutputCompleted)(
	struct SubstanceHandle_ *handle,
	unsigned int outputIndex,
	size_t jobUserData);


/** @typedef void SubstanceCallbackOutOfMemory
	@brief Type of the 'out of memory' callback function

	@code void (*)(SubstanceHandle* handle,int memoryType) @endcode

	This callback is called when Substance runs out of memory, i.e. when
	Substance cannot honor a memory budget OR an out of memory error is reported 
	by the platform.
	
	The 'handle' parameter is the Substance handle that calls this callback
	func.<BR>
	The 'memoryType' parameter is the memory concerned, as enumerated in the 
	SubstanceMallocExFlag type: 0 = system memory, 0x10 = "GPU" memory.*/
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackOutOfMemory)(
	struct SubstanceHandle_ *handle,
	int memoryType);


/** @typedef void SubstanceCallbackJobCompleted
	@brief Type of the 'render job completed' callback function

	@code void (*)(SubstanceHandle* handle,size_t jobUserData) @endcode

	This callback is called when Substance finishes a job and is about to
	start the next job in the render list.
	
	The 'handle' parameter is the Substance handle that calls this callback
	func.<BR>
	'jobUserData' is the integer/pointer set by the user at the creation of
	this job. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackJobCompleted)(
	struct SubstanceHandle_ *handle,
	size_t jobUserData);


/**	@typedef void* SubstanceCallbackMalloc
	@brief Type of the 'system memory allocation' callback function

	@code void* (*)(size_t bytesCount,size_t alignment) @endcode

	This callback allows to use user-defined dynamic memory allocation.
	
	The 'bytesCount' parameter is the number of bytes to allocate.<BR>
	The 'alignment' parameter is the required buffer address alignment.<BR>
	Must return the corresponding buffer. */
typedef void* (SUBSTANCE_CALLBACK *SubstanceCallbackMalloc)(
	size_t bytesCount,
	size_t alignment);


/** @typedef void SubstanceCallbackFree
	@brief Type of the 'system memory de-allocation' callback function

	@code void (*)(void* bufferPtr) @endcode

	This callback allows to free buffers allocated with the
	SubstanceCallbackMalloc callback.
	
	The 'bufferPtr' parameter is the buffer to free. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackFree)(
	void* bufferPtr);

	
/** @typedef void* SubstanceCallbackMallocEx
	@brief Type of the 'extended system memory allocation' callback function
	@warning This callback must be implemented on some platforms. Please see 
		the platform specific Substance documentation.

	@code void* (*)(
		unsigned int flags,
		size_t bytesCount,
		size_t alignment) @endcode

	This callback extend the SubstanceCallbackMalloc callback. In addition, 
	it allows the allocation of platform specific memory buffer types.

	The 'flags' parameter is a combination of 'SubstanceMallocExFlag' enums.
	Specifies the platform memory type to allocate.<BR> 
	The 'bytesCount' parameter is the number of bytes to allocate.<BR>
	The 'alignment' parameter is the required buffer address alignment.<BR>
	Must return the corresponding buffer.

	If this callback is not defined or it returns NULL, the 
	'SubstanceCallbackMalloc' callback is used instead. If the latter is not 
	defined too, default memory allocation is used. */
typedef void* (SUBSTANCE_CALLBACK *SubstanceCallbackMallocEx)(
	unsigned int flags,
	size_t bytesCount,
	size_t alignment);

/** @typedef void SubstanceCallbackFreeEx
	@brief Type of the 'extended memory de-allocation' callback function

	@code void (*)(unsigned int flags,void* bufferPtr) @endcode

	This callback allows to free buffers allocated with the
	SubstanceCallbackMallocEx callback.
	
	The 'flags' parameter is a combination of 'SubstanceMallocExFlag' enums.
	Specifies the platform memory type to de-allocate.<BR> 
	The 'bufferPtr' parameter is the buffer to free. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackFreeEx)(
	unsigned int flags,
	void* bufferPtr);
	
/** @typedef void* SubstanceCallbackInPlaceOutput
	@brief Type of the 'result output texture get buffer' callback function
	@warning Only valid with the BLEND Substance platform.

	@code void* (*)(
		struct SubstanceHandle_ *handle,
		size_t jobUserData,
		unsigned int outputIndex,
		unsigned int mipmapLevel,
		size_t *pitch,                    // Input, output
		size_t bytesCount,
		size_t alignment) @endcode

	This callback is called for each mipmap level every time an output texture 
	destination buffer is required.<BR>
	It allows the user to put the output in a specific (already allocated) 
	memory location.
	
	Textures allocated via this callback are NOT stored by Substance and NULL 
	pointers will be returned by the substanceHandleGetOutputs function 
	(see handle.h for further information).
	
	The 'handle' parameter is the Substance handle that calls this callback
	function.<BR>
	'jobUserData' is the user information of the job corresponding to this
	output.<BR>
	The 'outputIndex' parameter is the index of the completed output.<BR>
	The 'mipmapLevel' parameter is the mipmap level of the output 
	(0: base level).<BR>
	The 'pitch' parameter is a pointer to the size of a pixel row (or size of a 
	block row for compressed formats) in bytes. The user must modify it if a 
	non-default pitch is required. <BR>
	The 'bytesCount' parameter is the number of bytes to allocate. It is a 
	minimal value: the allocated buffer can actually be larger if the pitch is 
	tweaked by the user, in which case this value can be discarded. <BR>
	The 'alignment' parameter is the required buffer address alignment.<BR>
	Must return the corresponding buffer. This buffer will be accessed only for 
	write operations (no read operations: the buffer returned can be in mapped 
	video memory or equivalent). */
typedef void* (SUBSTANCE_CALLBACK *SubstanceCallbackInPlaceOutput)(
	struct SubstanceHandle_ *handle,
	size_t jobUserData,
	unsigned int outputIndex,
	unsigned int mipmapLevel,
	size_t *pitch,
	size_t bytesCount,
	size_t alignment);
	
/** @typedef void* SubstanceCallbackInputImageLock
	@brief Type of the 'lock input image' callback function

	@code void* (*)(
		struct SubstanceHandle_ *handle,
		size_t jobUserData,
		unsigned int inputIndex,
		struct SubstanceTextureInput_ **currentTextureInputDesc,      // IN/OUT
		const struct SubstanceTextureInput_ *preferredTextureInputDesc) @endcode

	This callback is called every time an input image texture/bitmap 
	(tweak entry) is required.<BR>
	It allows the user to: 
	  - Create the image just when it is required.
	  - Know what is the preferred size of the input (if the input image have a 
	    dynamic size, this is the only way to obtain it).
		
	See substanceHandleGetInputImageDesc for further information about 
	input preferred format descriptions.
	
	The 'handle' parameter is the Substance handle that calls this callback
	function.<BR>
	'jobUserData' is the user information of the job corresponding to the
	last 'substanceHandlePushSetInput' call that set this image.<BR>
	The 'inputIndex' parameter is the index of the input image.<BR>
	The 'currentTextureInputDesc' parameter is the pointer on pointer of the 
	texture description that will be use as input (can be NULL if no input 
	description have been pushed).<BR>
	The 'preferredTextureInputDesc' parameter is the pointer on the 
	input texture description that describe the preferred features of the
	image input (size, format, etc.). The mTexture member is not initialized. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackInputImageLock)(
	struct SubstanceHandle_ *handle,
	size_t jobUserData,
	unsigned int inputIndex,
	struct SubstanceTextureInput_ **currentTextureInputDesc,
	const struct SubstanceTextureInput_ *preferredTextureInputDesc);
	
/** @typedef void* SubstanceCallbackInputImageUnlock
	@brief Type of the 'unlock input image' callback function

	@code void* (*)(
		struct SubstanceHandle_ *handle,
		size_t jobUserData,
		unsigned int inputIndex,
		struct SubstanceTextureInput_ *currentTextureInputDesc) @endcode

	This callback is called every time an input image texture/bitmap 
	(tweak entry) is no more required.<BR>
	It allows the user to destroy the image just when it is no more required.
	
	The 'handle' parameter is the Substance handle that calls this callback
	function.<BR>
	'jobUserData' is the user information of the job corresponding to the
	last 'substanceHandlePushSetInput' call that set this image.<BR>
	The 'inputIndex' parameter is the index of the input image.<BR>
	The 'textureInputDesc' parameter is the pointer on the 
	texture description that was use as input (cannot be NULL this callback
	is NOT called when the input image is NULL). */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackInputImageUnlock)(
	struct SubstanceHandle_ *handle,
	size_t jobUserData,
	unsigned int inputIndex,
	struct SubstanceTextureInput_ *textureInputDesc);	


/** @typedef void SubstanceCallbackCacheEvict
	@brief Type of the 'cache evict/fetch' callback function

	@code void (*)(SubstanceHandle* handle,unsigned int flags,
		unsigned int itemUid,void buffer,size_t bytesCount) @endcode

	This callback is called by Substance Engine to evict, fetch AND/OR remove 
	intermediary results: cache eviction occurs when the current memory 
	consumption reach the memory budget.
	This callback is called for:
	  - Buffer eviction (Substance_CacheEvict_Evict): transfer to user.
	  - Buffer fetch (Substance_CacheEvict_Fetch): bring back to engine.
	  - Buffer remove (Substance_CacheEvict_Remove): cache item deprecated.
	
	The 'handle' parameter is the Substance handle that calls this callback
	func.<BR>
	The 'flags' parameter is a combination of SubstanceCacheEvictFlag enum.
	Describe the callback required actions. 'Evict' and 'Fetch' actions are 
	exclusives.<BR>
	The 'itemUid' parameter is the identifier of the cache item. This
	identifier is unique for this 'handle'.<BR>
	The 'buffer' parameter is a pointer on evicted data to read/write in
	function of the required action: if 'Fetch' action is required, this buffer
	must be filled w/ the content provided by the previous call w/ action
	'Evict' and same 'itemUid'. N.d. if the required action is only 'Remove'.<BR>
	The 'bytesCount' parameter is the size of the buffer in bytes. N.d. if the 
	required action is only 'Remove'. */
typedef void (SUBSTANCE_CALLBACK *SubstanceCallbackCacheEvict)(
	struct SubstanceHandle_ *handle,
	unsigned int flags,
	unsigned int itemUid,
	void *buffer,
	size_t bytesCount);


/** @typedef void* SubstanceCallbackProfileEvent
	@brief Type of the 'profile event' callback function
	@code void* (*)(
		struct SubstanceHandle_ *handle,
		size_t jobUserData,
		unsigned int eventType,
		unsigned int eventData) @endcode

	This callback is for internal use only. */
typedef void* (SUBSTANCE_CALLBACK *SubstanceCallbackProfileEvent)(
	struct SubstanceHandle_ *handle,
	size_t jobUserData,
	unsigned int eventType,
	unsigned int eventData);


#ifdef __cplusplus
}
/* End of the EXTERNC block (if necessary) */
#endif /* ifdef __cplusplus */

#endif /* ifndef _SUBSTANCE_CALLBACKS_H */
