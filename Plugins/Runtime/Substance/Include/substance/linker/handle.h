/** @file handle.h
	@brief Substance linker handle structure and related functions definitions
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance linker headers
	@date 20100406
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceLinkerHandle structure.
	Initialized from a SubstanceLinkerContext.
	This handle is used to set up linking parameters, select assemblies to
	link, launch the linking process and grab resulting binaries.<BR>

	Summary of functions defined in this file:

	Handle creation and deletion:
	  - substanceLinkerHandleInit
	  - substanceLinkerHandleRelease

	Assemblies list management:
	  - substanceLinkerHandlePushAssemblyEx (advanced options)
	  - substanceLinkerHandlePushAssemblyFileA (overload, ASCII filepath)
	  - substanceLinkerHandlePushAssemblyFileW (overload, Unicode filepath)
	  - substanceLinkerHandlePushAssemblyMemory (overload, buffer in memory)

	Outputs management:
	  - substanceLinkerHandleSelectOutputs (link assemblies subset)
	  - substanceLinkerHandleCreateOutputEx (shuffle channels, resize, flip)
	  - substanceLinkerHandleSetOutputFormat (change image output format)

	Advanced I/O manipulation:
	  - substanceLinkerConnectOutputToInput (connect an image input to an image output)
	  - substanceLinkerFuseInputs (join together two inputs)
	  - substanceLinkerConstifyInput (freeze and remove a numeric input)

	Linking process start and grab binary result (SBSBIN format):
	  - substanceLinkerHandleLink

	User data management (used in callbacks)
	  - substanceLinkerHandleSetUserData
	  - substanceLinkerHandleGetUserData

	Advanced usage:
	  - substanceLinkerHandleGetCacheMapping (grab cache mapping data)
	  - substanceLinkerHandleGetMetrics (grab some node metrics)

	Typical call sequence for Substance handle:
	  - 1. substanceLinkerHandleInit
	  - 2. substanceLinkerHandlePushAssembly...
	  - 3. substanceLinkerHandleLink
	  - 4. substanceLinkerHandleRelease
*/


#ifndef _SUBSTANCE_LINKER_HANDLE_H
#define _SUBSTANCE_LINKER_HANDLE_H


/** Context structure and related functions definitions */
#include "context.h"

/** Assembly push structures definition */
#include "push.h"

/** Create output structures definition */
#include "outputcreate.h"

/** Metrics structure */
#include "metrics.h"


/** @brief Selection flags.

	Used by the substanceLinkerHandleSelectOutputs 'action' parameter. */
typedef enum
{
	Substance_Linker_Select_Unselect    = 0x0, /**< Deselect one output */
	Substance_Linker_Select_Select      = 0x1, /**< Select one output */
	Substance_Linker_Select_UnselectAll = 0x2  /**< Deselect ALL outputs */

} SubstanceLinkerSelect;


/** @brief Union used to set input value (substanceLinkerConstifyInput) */
typedef union
{
	float fp[4];           /**< For floating point scalar or 2/3/4 vector */
	int integer[4];        /**< For integer scalar or 2/3/4 vector types inputs  */
	unsigned int uinteger; /**< For unsigned integer type inputs */

} SubstanceLinkerInputValue;



/** @brief Substance linker handle structure pre-declaration.

	Must be initialized using the substanceLinkerHandleInit function and
	released using the substanceLinkerHandleRelease function. */
struct SubstanceLinkerHandle_;

/** @brief Substance linker handle structure pre-declaration (C/C++ compatible).
*/
typedef struct SubstanceLinkerHandle_ SubstanceLinkerHandle;



/** @brief Substance linker handle initialization from context
	@param[out] substanceLinkerHandle Pointer to the Substance Linker handle to
		initialize.
	@param substanceLinkerContext Pointer to the Substance Linker context used
		for the linking of substance data.
	@pre The handle must be previously un-initialized or released.
	@pre The context must be valid (correctly initialized) and remain valid
		during the handle lifetime.
	@post The handle is now valid (if no errors occurred).
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandleInit(
	SubstanceLinkerHandle** substanceLinkerHandle,
	SubstanceLinkerContext* substanceLinkerContext);

/** @brief Substance linker handle release
	@param substanceLinkerHandle Pointer to the Substance handle to release.
	@pre The handle must be already initialized and there must be no linking
		processes currently running.
	@post This handle is not valid any more (it can be re-initialized).
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandleRelease(
	SubstanceLinkerHandle* substanceLinkerHandle);


/** @brief Add an assembly (OR an archive) to link, extended version
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param src Pointer on a structure (defined in ./push.h) that describes the
		assembly to link (SBSASM file) OR filepath to archive (SBSAR file) which
		contains one or several SBSASM to link.
	@pre The handle must be previously initialized.
	@return Returns 0 if succeeded, an error code otherwise.

	A call to this function adds an assembly to link. All outputs in this
	assembly are added to the output results list, until they are explicitly
	unselected via substanceLinkerHandleSelectOutputs OR until it is called in
	SubstanceLinkerCallbackDependency context.
	If 'src' references a SBSAR format file, each SBSASM contained in this
	archive is added to link. In addition, the SubstanceLinkerCallbackArchiveXml
	callback (see ./callbacks.h) is called for each XML description file
	contained in the archive.

	At least one of the substanceLinkerHandlePushAssembly... function must be
	called before linking (substanceLinkerHandleLink) */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandlePushAssemblyEx(
	SubstanceLinkerHandle* substanceLinkerHandle,
	const SubstanceLinkerPush* src);

/** @brief Add an assembly (OR an archive) FILE to link, ASCII filepath version
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param filepath ASCII null-terminated string.
	@pre The handle must be previously initialized.
	@return Returns 0 if succeeded, an error code otherwise.

	substanceLinkerHandlePushAssemblyEx overload, provided for convenience. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandlePushAssemblyFileA(
	SubstanceLinkerHandle* substanceLinkerHandle,
	const char* filepath);

/** @brief Add an assembly FILE (OR an archive) to link, UNICODE filepath
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param filepath  UNICODE null-terminated string.
	@pre The handle must be previously initialized.
	@return Returns 0 if succeeded, an error code otherwise.

	substanceLinkerHandlePushAssemblyEx overload, provided for convenience. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandlePushAssemblyFileW(
	SubstanceLinkerHandle* substanceLinkerHandle,
	const wchar_t* filepath);

/** @brief Add an assembly IN MEMORY to link
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param ptrData Pointer on a buffer that contains the assembly to link.
		This buffer is NOT copied.
	@param sizeData Size of the buffer pointed by 'ptrData'. Only used if
		the buffer contains an archive (SBSAR format).
	@warning If ptrData references an assembly (SBSASM format), the buffer must
		be valid until calling substanceLinkerHandleLink.
	@pre The handle must be previously initialized.
	@return Returns 0 if succeeded, an error code otherwise.

	substanceLinkerHandlePushAssemblyEx overload, provided for convenience. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandlePushAssemblyMemory(
	SubstanceLinkerHandle* substanceLinkerHandle,
	const char* ptrData,
	size_t sizeData);


/** @brief Select a subset of outputs to link
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param action Flag that indicates the action to perform: select output,
		deselect output or deselect all.
	@param outputUID The UID of the output to select or deselect (not used if
		action is Substance_Linker_Select_UnselectAll).
	@pre The handle must be previously initialized.
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandleSelectOutputs(
	SubstanceLinkerHandle* substanceLinkerHandle,
	SubstanceLinkerSelect action,
	unsigned int outputUID);

/** @brief Create a new modified output from existing outputs
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param[out] outputUID Returns the UID of the created output.
	@param options Description of new output options (see createoutput.h).
	@pre The handle must be previously initialized and assemblies that contains
		outputs referenced by this function must be already pushed.
	@return Returns 0 if succeeded, an error code otherwise.

	This function can create a new output by shuffling channels (R,G,B,A)
	of existing outputs. It allows to adapt textures channels distribution in
	function of requirements (e.g. specular put into diffuse alpha channel,
	specific normal map layout). Also, it allows to remap channel values
	(e.g. inverse normal map axes), force output size and
	flip result horizontally and/or vertically. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandleCreateOutputEx(
	SubstanceLinkerHandle* substanceLinkerHandle,
	unsigned int *outputUID,
	const SubstanceLinkerOutputCreate *options);

/** @brief Create a new output by shuffling channels of existing outputs
	@deprecated substanceLinkerHandleCreateOutputEx should be instead (more
		creation options).
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param[out] outputUID Returns the UID of the created output.
	@param format Substance pixel format of the created output.
		Combination of SubstancePixelFormat enums (see substance/pixelformat.h
		for the complete list).
	@param mipmapLevelsCount Mipmap pyramid depth of the created output
		(0: full mipmap pyramid, 1: no mipmaps)
	@param hvFlip Horizontal and vertical flipping: one of SubstanceLinkerHVFlip
		enumeration (see outputcreate.h)
	@param channels Description of new output channels make-up
		(see outputshuffle.h)
	@pre The handle must be previously initialized and assemblies that contains
		outputs referenced by this function must be already pushed.
	@return Returns 0 if succeeded, an error code otherwise.

	See substanceLinkerHandleCreateOutputEx. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandleCreateOutput(
	SubstanceLinkerHandle* substanceLinkerHandle,
	unsigned int *outputUID,
	unsigned int format,
	unsigned int mipmapLevelsCount,
	SubstanceLinkerHVFlip hvFlip,
	const SubstanceLinkerShuffle *channels);

/** @brief Change image output format and mipmap pyramid depth
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param outputUID UID of the output to change format.
	@param format New Substance pixel format.
		Combination of SubstancePixelFormat enums (see substance/pixelformat.h
		for the complete list).
	@param mipmapLevelsCount Mipmap pyramid depth (0: full mipmap
		pyramid, 1: no mipmaps). Set to -1 to keep current value.
	@pre The handle must be previously initialized and assembly that contains
		the output referenced by this function must be already pushed and have
		an image type..
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandleSetOutputFormat(
	SubstanceLinkerHandle* substanceLinkerHandle,
	unsigned int outputUID,
	unsigned int format,
	int mipmapLevelsCount);


/** @brief Connect an output of an assembly to the input of an other assembly
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param inputUID UID of the input to connect to
	@param outputUID UID of the output to connect from
	@pre The handle must be previously initialized and assemblies that contains
		the output and input referenced by this function must be already pushed.
		The input and the output must be in two distinct assemblies. Cyclic
		dependencies between assemblies are not allowed. Also, the input must
		be not already connected. The output must have image format.
	@post If succeed, both input and outputs are connected and are no more
		visible at runtime.
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerConnectOutputToInput(
	SubstanceLinkerHandle* substanceLinkerHandle,
	unsigned int inputUID,
	unsigned int outputUID);

/** @brief Fuse two inputs together (w/ same type)
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param input0UID UID of the first input to join (kept).
	@param input1UID UID of the second input to join (concealed).
	@pre The handle must be previously initialized and assemblies that contains
		the inputs referenced by this function must be already pushed.
		The inputs must have the same type.
		The input0UID and input1UID parameters must be not equal.
		The inputs must be not disabled by another fuse or connect call.
	@post If succeed, both inputs are fused together. The second input is
		hidden: its UID becomes invalid, only the first input will be visible.
		Also, the default value of the second input is discarded.
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerFuseInputs(
	SubstanceLinkerHandle* substanceLinkerHandle,
	unsigned int input0UID,
	unsigned int input1UID);

/** @brief Freeze and remove a numeric input
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param inputUID UID of the input to freeze.
	@param value Pointer on the numeric value used as a replacement of input.
		Can be NULL: the input default value is used. The value type must match
		the input type (see substance/inputdesc.h).
	@pre The handle must be previously initialized and assemblies that contains
		the input referenced by this function must be already pushed.
	@post If succeed, the input is hidden: its UID becomes invalid.
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerConstifyInput(
	SubstanceLinkerHandle* substanceLinkerHandle,
	unsigned int inputUID,
	const SubstanceLinkerInputValue* value);


/** @brief Linking process: process the pushed assemblies and grab binary result
	@param substanceLinkerHandle Pointer to the Substance handle to link.
	@param[out] resultData Sets the pointer on the binary string result (SBSBIN).
		Set to NULL if failed. Remains valid during the handle lifetime and
		until the next substanceLinkerHandleLink call on the same handle.
	@param[out] resultSize Sets the data size of the binary string result.
		Set to 0 if failed. Set NULL if result size is not required.
	@pre The handle must be previously initialized, at least one assembly must
		be pushed.
	@post The assemblies list is reseted if linking succeed
	@warning Binary result is specific to linker version, target engine
		platform (set at context creation) and OS/Architecture (see options.h).
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandleLink(
	SubstanceLinkerHandle* substanceLinkerHandle,
	const unsigned char** resultData,
	size_t* resultSize);


/** @brief Set the user data associate to the handle
	@param substanceLinkerHandle Pointer to the Substance handle to set data.
	@param userData User data (integer/pointer) to associate with the handle.
		This value can be grabbed in callback functions (by using
		substanceLinkerHandleGetUserData). User data is provided for convenience
		only, this value is not interpreted by the linker.
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandleSetUserData(
	SubstanceLinkerHandle* substanceLinkerHandle,
	size_t userData);

/** @brief Get the user data associate to the handle
	@param substanceLinkerHandle Pointer to the Substance handle to set data.
	@param[out] userData Sets the User data associated with the handle w/
		substanceLinkerHandleSetUserData (user data default value is 0).
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandleGetUserData(
	SubstanceLinkerHandle* substanceLinkerHandle,
	size_t* userData);


/** @brief Grab cache mapping data of the latest linking process
	@param substanceLinkerHandle Pointer to the Substance handle.
	@param[out] cacheData Sets the pointer on cache mapping data.
		Set to NULL if failed. Remains valid during the handle lifetime and
		until the next substanceLinkerHandleGetCacheMapping call on the same
		handle.
	@param[out] cacheSize Sets the size of cache mapping data. Set to 0 if
		failed. Set to NULL if size is not required.
	@param previousCacheData Pointer on cache mapping data returned in a
		previous call or set to NULL if not available.
	@pre The handle must be previously initialized, at least one link must
		be completed.
	@return Returns 0 if succeeded, an error code otherwise.

	Cache mapping data is used by the Substance Engine to switch from a
	previously linked SBSBIN to an updated SBSBIN without loosing internal
	cache items. See the Engine function substanceHandleTransferCache in
	../handle.h.

	Only assemblies/archives pushed with a non-zero uid are referenced in
	cache mapping data (SubstanceLinkerPush::uid). This uid allows to identify
	each pushed assembly, even if the same assembly is pushed several times.

	This system allows the user to link all required assemblies in one SBSBIN
	and later link a new set of assemblies without performance issues (without
	loosing the internal cache of kept assemblies). */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandleGetCacheMapping(
	SubstanceLinkerHandle* substanceLinkerHandle,
	const unsigned char** cacheData,
	size_t* cacheSize,
	const unsigned char* previousCacheData);


/** @brief Get a metrics report for a given handle
    @param      substanceLinkerHandle Pointer to the Substance handle to query
    @param[out] pMetrics Pointer to the metrics struct to fill.
    @return     Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerHandleGetMetrics(
	SubstanceLinkerHandle*        substanceLinkerHandle,
	SubstanceLinkerHandleMetrics* pMetrics);

#endif /* ifndef _SUBSTANCE_LINKER_HANDLE_H */
