/** @file handle.h
	@brief Substance engine handle structure and related functions definitions
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080107
	@copyright Allegorithmic. All rights reserved.
	@version 1.8
		- Numeric outputs added.
		- SubstanceIOType renamed as SubstanceInputType.
		- substanceHandleGetOutputs is now internal.
		- substanceHandleGetImageOutput function added.
		- substanceHandleGetNumericOutput function added.

	Defines the SubstanceHandle structure. Initialized from a SubstanceContext
	and a buffer that contains the Substance cooked data to render.
	This handle is used to set up generation parameters, select textures to
	render, fill-in inputs, launch the generation process and grab resulting
	textures.<BR>
	The rendering process can be launched synchronously or asynchronously (in
	one go or using time-slicing). The process workload can be dispatched on
	the different hardware units.<BR>
	Generated textures can be grabbed progressively using a callback
	function or at a stretch at the end of generation.

	Summary of functions defined in this file:

	Handle creation and deletion:
	  - substanceHandleInit
	  - substanceHandleRelease

	Render list management:
	  - substanceHandlePushOutputs
	  - substanceHandlePushSetInput (advanced usage)
	  - substanceHandleFlush (advanced usage)

	Generation process start/stop, resources switch, and state accessor:
	  - substanceHandleStart
	  - substanceHandleStop (advanced usage)
	  - substanceHandleSwitchHard (advanced usage)
	  - substanceHandleGetState (advanced usage)

	Grab Results:
	  - substanceHandleGetImageOutput
	  - substanceHandleGetNumericOutput

	Substance data description accessors:
	  - substanceHandleGetDesc
	  - substanceHandleGetOutputDesc
	  - substanceHandleGetInputDesc (advanced usage)
	  - substanceHandleGetInputImageDesc (advanced usage)

	Synchronisation:
	  - substanceHandleSyncSetLabel (advanced usage)
	  - substanceHandleSyncGetLabel (advanced usage)

	Cache transfer:
	  - substanceHandleTransferCache (advanced usage)

	User data management:
	  - substanceHandleSetUserData (advanced usage)
	  - substanceHandleGetUserData (advanced usage)

	Typical call sequence for Substance handle (synchronous, grab all outputs):
	  - 1. substanceHandleInit (initialization from Substance cooked data)
	  - 2. substanceHandleGetDesc (retrieve the number of outputs)
	  - 3. substanceHandlePushOutputs (request the needed outputs)
	  - 4. substanceHandleStart (launch the rendering process)
	  - 5. substanceHandleGetXXXXOutput (grab the outputs textures|values)
	  - 6. substanceHandleRelease (close the handle)
*/

#ifndef _SUBSTANCE_HANDLE_H
#define _SUBSTANCE_HANDLE_H


/** Context structure and related functions definitions */
#include "context.h"

/** Texture (results) structure and related functions definitions */
#include "texture.h"

/** Callbacks signatures definitions */
#include "callbacks.h"

/** Substance data description definitions */
#include "datadesc.h"

/** Output description definitions */
#include "outputdesc.h"

/** Input description definitions */
#include "inputdesc.h"

/** Texture used as input, contains platform specific texture and format info */
#include "textureinput.h"

/** Hardware resource dispatching requirements definitions */
#include "hardresources.h"

/** Platform dependent definitions */
#include "platformdep.h"

/** Error codes list */
#include "errors.h"

/** Enumerations used by handle functions */
#include "enums.h"


/** @brief Substance engine handle structure pre-declaration.

	Must be initialized using the substanceHandleInit function and released
	using the substanceHandleRelease function. */
struct SubstanceHandle_;

/** @brief Substance engine handle structure pre-declaration (C/C++ compatible).
*/
typedef struct SubstanceHandle_ SubstanceHandle;



/** @brief Substance engine handle initialization from context and data
	@param[out] substanceHandle Pointer to the Substance handle to initialize.
	@param substanceContext Pointer to the Substance context used for the
		rendering of substance data.
	@param substanceDataPtr Pointer to the substance cooked data to render.
	@param substanceDataSize Size of the substanceDataPtr buffer in bytes.
		Used only for debug purposes. Can be 0 (no overflow checks can be done).
	@param[in] substanceRscInitial Pointer to initial hardware resource
		dispatching. Set this parameter to NULL to use the default parameters
		of the engine for the current platform. If provided, allows to:
		  - Limit memory use per device.
		  - Choose CPU/GPU/SPU affinity.
		  - Suspend computation.
	@pre The handle must be previously un-initialized or released.
	@pre The context must be valid (correctly initialized) and remain valid
	during the handle lifetime.
	@pre The Substance data must be present and valid during the handle
		lifetime. It must be at least 16 BYTES ALIGNED.
	@post The handle is now valid (if no errors occurred).
	@note If the context is 'read-only' (see context.h), this handle cannot be
		used for rendering, only substanceHandleGet* functions can be used.
	@return Returns 0 if succeeded, an error code otherwise.

	Dynamic allocations of intermediate scratch buffers are done in this
	function.
	This function is quite fast (## to evaluate ##) and does not use
	API/hardware related resources (i.e. it is GPU-calls-free). */
SUBSTANCE_EXPORT unsigned int substanceHandleInit(
	SubstanceHandle** substanceHandle,
	SubstanceContext* substanceContext,
	const unsigned char* substanceDataPtr,
	size_t substanceDataSize,
	const SubstanceHardResources* substanceRscInitial);

/** @brief Substance engine handle release
	@param substanceHandle Pointer to the Substance handle to release.
	@pre The handle must be already initialized and there must be no generation
		processes currently running.
	@post This handle is not valid any more (it can be re-initialized).
		Non-grabbed result textures are released.
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceHandleRelease(
	SubstanceHandle* substanceHandle);




/** @brief Push a new set of outputs in the render list.
	@param substanceHandle Pointer to the Substance handle.
	@param flags Generation flags. A combination of SubstancePushOption and
		SubstanceOutputSelectionOption flags. These flags allow to choose the
		selectList parameter behavior and to set the hint flag.
	@param[in] selectList Pointer to the array of textures to render. Can be
		NULL: all data outputs are selected. Otherwise the array is interpreted
		according to the 'flags' parameter value:
		<UL>
			<LI>Substance_OutOpt_OutIndex (default): the list of integers is
			interpreted as output indices.</LI>
			<LI>Substance_OutOpt_OutputId: list of output unique
			identifiers.</LI>
		</UL>
		By default, the selectList data is copied by Substance. If the data is
		persistent (always present in memory), set the
		Substance_OutOpt_DoNotCopy flag to avoid copy.
	@param selectListCount Number of entries in the selectList array.
	@param jobUserData User data (integer/pointer) to associate with the job.
		This value is transmitted to callback functions: it allows tracking of
		the job lifetime. This parameter is optional, e.g. set to 0 by default.
	@pre The handle must be previously initialized.
	@pre If the Substance_OutOpt_DoNotCopy flag is set, the selectList buffer
		must remain valid during the job lifetime.
	@return Returns 0 if succeeded, an error code otherwise.

	A call to this function describes a job. This job is added at the end of the
	render list. Pending jobs are not affected.

	If the Substance_PushOpt_HintOnly flag is set, the outputs are pushed into
	the render list as usual, but they will not be rendered. This feature allows
	the game engine to send hints to Substance (based on the game level design
	and/or the player behavior). These hints allow Substance to anticipate
	future computations and manage internal pools accordingly.<BR>
	Hints must be pushed at the end of the render list: Non-hints jobs cannot be
	appended after hints jobs.

	The outputs of the list are not rendered in the order of the list. However
	the jobs of the render list are processed sequentially without any
	reordering.

	If a output index is out of range or an Id is invalid, an error is returned
	and the whole command is ignored. */
SUBSTANCE_EXPORT unsigned int substanceHandlePushOutputs(
	SubstanceHandle* substanceHandle,
	unsigned int flags,
	const unsigned int selectList[],
	unsigned int selectListCount,
	size_t jobUserData);

/** @brief Set input (render param.) value. Action appended in the render list.
	@param substanceHandle Pointer to the Substance handle.
	@param flags Combination of SubstancePushOption flags. These flags allow to
		choose the way this item is added to the render list and the hint flag.
	@param inIndex Index of the input to set the value for.
	@param inType Type of the value
	@param value Pointer to value. Can be NULL if Substance_PushOpt_HintOnly
		flag is set.<BR>
		The concrete type of the value depends on inType:
		  - Substance_IOType_Float, Substance_IOType_Float2/3/4:
		    array of 1/2/3/4 float(s).
		  - Substance_IOType_Integer, Substance_IOType_Integer2/3/4:
		    array of 1/2/3/4 integer(s).
		  - Substance_IOType_Image: a pointer to a SubstanceTextureInput
		    structure or NULL (no image set or provided just in time via
		    InputImageLock/Unlock callbacks).
		    Calling function must ensure the memory stays valid for all
		    subsequent calls to substanceHandleStart.
		    i.e. end of substanceHandleStart that use this input.
		  - Substance_IOType_String: an NULL-terminated uint32 array that
		    contains the string in UCS4 (UTF-32) encoding.
		    Calling function must ensure the memory stays valid for all
		    subsequent calls to substanceHandleStart.
		  - Substance_IOType_Font: a pointer on SubstanceInputFont structure
		    correctly filled. Font content buffer is deep copied and can be
		    discarded just after this call.
	@param jobUserData User data (integer/pointer) to associate with the render
		list item. This value is transmitted to callback functions: it allows
		tracking of the render list item lifetime. This parameter is optional,
		e.g. set to 0 by default.
	@pre The handle must be previously initialized and valid.
	@pre The input index must be valid (< inputCount).
	@pre The concrete type of the input and inType must match. Except if the
		Substance_PushOpt_HintOnly flag is set and value is NULL.
	@return Returns 0 if succeeded, an error code otherwise.

	A call to this function does not modify the input immediately, but pushes a
	new item in the render list. Like substanceHandlePushOutputs, this function
	handles the Substance_PushOpt_HintOnly flag (see substanceHandlePushOutputs
	description for further information). */
SUBSTANCE_EXPORT unsigned int substanceHandlePushSetInput(
	SubstanceHandle* substanceHandle,
	unsigned int flags,
	unsigned int inIndex,
	SubstanceIOType inType,
	void* value,
	size_t jobUserData);

/** @brief Flush the render list.
	@param substanceHandle Pointer to the Substance handle.
	@return Returns 0 if succeeded, an error code otherwise.

	Flush ALL the items previously pushed in the render list. The process is not
	stopped: if other jobs are pushed just after the flush commands, the process
	proceed to these new jobs.

	The internal cache is NOT flushed by this function. */
SUBSTANCE_EXPORT unsigned int substanceHandleFlush(
	SubstanceHandle* substanceHandle);




/** @brief Computation process launch, process the render list
	@param substanceHandle Pointer to the Substance handle to compute.
	@param flags Combination of SubstanceStartOption and SubstanceSyncOption
		enumerations.<BR>
		Set Substance_Sync_Asynchronous to launch the process in another thread
		and to return immediately (does not wait for process termination).<BR>
		Set Substance_Ending_Waiting to force asynchronous process waiting for
		further computation at render list ending. When this flag is set, an
		explicit stop (substanceHandleStop) is needed. Ignored if
		Substance_Sync_Asynchronous flag is not set.
	@pre The handle must be previously initialized.
	@return Returns 0 if succeeded, an error code otherwise.<BR>
		Returns immediately if asynchronous mode is selected.<BR>
		Returns at the end of computation otherwise, i.e.:
		  - The end of the render list is reached
		  - Through substanceHandleStop call (from another thread or through
		  a callback function).
		  - Current/switched hardware resource dispatching does not allocate any
		  process unit (see substanceHandleSwitchHard function).
		  - Return immediately w/ Substance_Error_DeviceLost error code if
		  the platform specific hardware device is lost.

	This function is used to consume the render list. */
SUBSTANCE_EXPORT unsigned int substanceHandleStart(
	SubstanceHandle* substanceHandle,
	unsigned int flags);

/** @brief Cancel/Stop the computation process.
	@param substanceHandle Pointer to the Substance handle.
	@param syncMode Set Substance_Sync_Asynchronous to return immediately (does
		not wait for the actual process termination).
	@pre The handle must be previously initialized and valid.
	@return Returns 0 if succeeded, an error code otherwise.<BR>
		Returns immediately if syncMode is Substance_Sync_Asynchronous or the
		current process is launched synchronously (allows call through a
		callback function or from another thread).<BR>
		Returns at actual computation end otherwise.

	Stops the computation process. DOES NOT flush the render list.<BR>
	This function can be called at any time for processes started both in
	asynchronous or synchronous mode. */
SUBSTANCE_EXPORT unsigned int substanceHandleStop(
	SubstanceHandle* substanceHandle,
	SubstanceSyncOption syncMode);

/** @brief On the fly switch to another hardware resource dispatching
	@param substanceHandle Pointer to the Substance handle.
	@param syncMode Synchronous flag, handled differently depending on whether
		the duration is finite or not, a current process is running or not and
		this process is synchronous or not (see the returned value description).
	@param[in] substanceRscNew Pointer to new hardware resource dispatching
		requirements and hints. Set this parameter to NULL in order to use
		the initial resource dispatching given at the substanceHandleInit
		function call (or platform default ones if not given).
	@param[in] substanceRscNext Pointer to hardware resource dispatching used
		at window time end (see the 'duration' parameter). Set this parameter
		to NULL in order to use initial resource dispatching
		(see substanceRscNew). Meaningful only if duration is finite.
	@param duration Duration of the switch in microseconds. Can be 0: INFINITE.
	@pre The handle must be previously initialized and valid.
	@return Returns 0 if succeeded, an error code otherwise.<BR>
		<TABLE><CAPTION>Return behavior</CAPTION>
		<TR>
			<TD><I>Parameters --></I></TD>
			<TD><B>Finite/Synchronous</B></TD>
			<TD><B>Finite/Asynchronous</B></TD>
			<TD><B>Infinite/Synchronous</B></TD>
			<TD><B>Infinite/Asynchronous</B></TD>
		</TR>
		<TR>
			<TD><B>No process running</B></TD>
			<TD>Returns immediately (1)</TD>
			<TD>Returns immediately (1)</TD>
			<TD>Returns immediately (1)</TD>
			<TD>Returns immediately (1)</TD>
		</TR>
		<TR>
			<TD><B>Asynchronous process</B></TD>
			<TD>Returns at duration end</TD>
			<TD>Returns immediately</TD>
			<TD>Returns when the switch is effective</TD>
			<TD>Returns immediately</TD>
		</TR>
		<TR>
			<TD><B>Synchronous process</B></TD>
			<TD>Returns immediately (2)</TD>
			<TD>Returns immediately (2)</TD>
			<TD>Returns immediately (2)</TD>
			<TD>Returns immediately (2)</TD>
		</TR></TABLE>
		(1): Substance consider this hardware resource switch only on next
			substanceHandleStart call. In some cases (e.g. for flushing memory)
			you have to call substanceHandleStart just to perform the
			switch.<BR>
		(2): Allows call through a callback function or from another thread.

	This function allows to:
	  - Limit memory use per device (e.g. flush video memory).
	  - Choose CPU/GPU/SPU affinity.
	  - Suspend computation: The process can be put in standby if the
	    SubstanceHardResources does not allocate any process unit.

	This function allows to change the hardware resource dispatching during
	or before a computation (e.g. use a GPU between vsync idle time:
	time-slicing strategy).<BR>
	The change is not limited in time if 'duration' == 0 (INFINITE).
	Otherwise, the switch is clocked: 'duration' defines a time range in
	microseconds. */
SUBSTANCE_EXPORT unsigned int substanceHandleSwitchHard(
	SubstanceHandle* substanceHandle,
	SubstanceSyncOption syncMode,
	const SubstanceHardResources* substanceRscNew,
	const SubstanceHardResources* substanceRscNext,
	unsigned int duration);

/** @brief Get the computation process state
	@param substanceHandle Pointer to the Substance handle to get the state
		from.
	@param[out] jobUserData If not NULL, returns the user data associated with
		the render list item currently processed. Returns 0 if no process is
		currently running.
	@param[out] state If not NULL, returns an integer containing the current
		state: combination of SubstanceStateFlag flags.
	@pre The handle must be previously initialized and valid.
	@return Returns 0 if succeeded, an error code otherwise.

	This function can be called at any time for processes started both in
	asynchronous or synchronous mode. */
SUBSTANCE_EXPORT unsigned int substanceHandleGetState(
	const SubstanceHandle* substanceHandle,
	size_t *jobUserData,
	unsigned int* state);


/** @brief Grab output result of type texture
	@param substanceHandle Pointer to the Substance handle to grab texture from.
	@param flags One of SubstanceOutputSelectionOption flags (only
		Substance_OutOpt_OutIndex or Substance_OutOpt_OutputId flags are
		supported). Set to 0 to simply grab textures from its index numbers.
	@param outIndexOrId Index of the output to grab or Id of the texture
		to grab if the Substance_OutOpt_OutputId flag is set.
	@param[out] substanceTexture Pointer to the texture structure to fill.
	@pre The handle must be previously initialized and valid.
	@pre The output index must be valid.
	@pre The output must be an image (Substance_IOType_Image).
	@post The corresponding output is transferred to the caller. The ownership
		is also transferred: the user have to free/release the texture when
		necessary.
	@note Is it not necessary to use this function for the output textures
		allocated using the SubstanceCallbackInPlaceOutput callback: these
		textures are NOT stored by Substance and NULL pointers will be returned
		by this function.
	@warning Result texture is NOT counted in the memory budget of the
		Substance handle. Memory budget must be adjusted if necessary.
	@return Returns 0 if succeeded, an error code otherwise (returns
		Substance_Error_NotComputed if the output is not already computed).

	This function can be called at any time, even during a computation. */
#define substanceHandleGetImageOutput(substanceHandle,flags,outIndexOrId,substanceTexture) \
	substanceHandleGetOutputs( \
		substanceHandle, \
		flags, \
		outIndexOrId, \
		1u, \
		substanceTexture)

/** @brief Grab output result of type numerical
	@param substanceHandle Pointer to the Substance handle to grab output from.
	@param flags One of SubstanceOutputSelectionOption flags (only
		Substance_OutOpt_OutIndex or Substance_OutOpt_OutputId flags are
		supported). Set to 0 to simply grab outputs from its index numbers.
	@param outIndexOrId Index of the output to grab or Id of the output
		to grab if the Substance_OutOpt_OutputId flag is set.
	@param[out] value Pointer to the numerical value to fill.
	@pre The handle must be previously initialized and valid.
	@pre The output index must be valid.
	@pre The output must have a numerical type.
	@return Returns 0 if succeeded, an error code otherwise (returns
		Substance_Error_NotComputed if the output is not already computed).

	This function can be called at any time, even during a computation. */
#define substanceHandleGetNumericalOutput(substanceHandle,flags,outIndexOrId,value) \
	substanceHandleGetOutputs( \
		substanceHandle, \
		flags|Substance_OutOpt_Numerical, \
		outIndexOrId, \
		1u, \
		value)


/** @brief Get the handle main description
	@param[in] substanceHandle Pointer to the Substance handle of which to get
		the description.
	@param[out] substanceInfo Pointer to the information structure to fill.
	@pre The handle must be previously initialized and valid.
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceHandleGetDesc(
	const SubstanceHandle* substanceHandle,
	SubstanceDataDesc* substanceInfo);

/** @brief Get an output (texture or numerical result) description and state
	@param[in] substanceHandle Pointer to the Substance handle.
	@param outIndex Index of the output of which to get the description.
	@param[out] substanceOutDesc Pointer to the output information struct to
		fill.
	@pre The handle must be previously initialized and valid.
	@pre The output index must be valid (< outputCount).
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceHandleGetOutputDesc(
	const SubstanceHandle* substanceHandle,
	unsigned int outIndex,
	SubstanceOutputDesc* substanceOutDesc);

/** @brief Get an input (render parameter) description
	@param[in] substanceHandle Pointer to the Substance handle.
	@param inIndex Index of the input of which to get the description.
	@param[out] substanceInDesc Pointer to the input information struct to fill.
	@pre The handle must be previously initialized and valid.
	@pre The input index must be valid (< inputCount).
	@return Return 0 if succeed, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceHandleGetInputDesc(
	const SubstanceHandle* substanceHandle,
	unsigned int inIndex,
	SubstanceInputDesc* substanceInDesc);

/** @brief Get an image input (render entry) preferred format description
	@param[in] substanceHandle Pointer to the Substance handle.
	@param inIndex Index of the image input of which to get the format
		description. This index MUST correspond to a Substance_IOType_Image
		type input.
	@param[out] substanceTextureInDesc Pointer to the texture input description
		struct to fill. All members are filled except the mTexture member.
	@pre The handle must be previously initialized and valid.
	@pre The input index must be valid (< inputCount) and correspond to a
		Substance_IOType_Image type input.
	@return Return 0 if succeed, an error code otherwise.

	This function is used to get the preferred format and size of an image
	input. It returns the expected texture/bitmap features. */
SUBSTANCE_EXPORT unsigned int substanceHandleGetInputImageDesc(
	const SubstanceHandle* substanceHandle,
	unsigned int inIndex,
	SubstanceTextureInput* substanceTextureInDesc);


/** @brief Synchronization function: set the handle synchronization label value
    @param substanceHandle Pointer to the Substance handle.
    @param labelValue The value to assign to the handle synchronization label.
		This value will be really set when this call is processed by the
		Substance internal command buffer loop.
    @return Returns 0 if succeeded, an error code otherwise.

	The synchronization label value can be accessed with the
	substanceHandleSyncGetLabel function.<BR>
	This function allows the user to check if previous commands were already
	processed by the Substance handle. It can be used for discarding deprecated
	callbacks. */
SUBSTANCE_EXPORT unsigned int substanceHandleSyncSetLabel(
    SubstanceHandle* substanceHandle,
    size_t labelValue);

/** @brief Synchronization function: accessor to the synchronization label value
    @param substanceHandle Pointer to the Substance handle.
    @param[out] labelValuePtr Pointer to the variable that will receive the
		label value.
    @return Returns 0 if succeeded, an error code otherwise.

	The synchronization label value is modified with the
	substanceHandleSyncSetLabel function.<BR>
	The default label value is 0. */
SUBSTANCE_EXPORT unsigned int substanceHandleSyncGetLabel(
    SubstanceHandle* substanceHandle,
    size_t* labelValuePtr);


/** @brief Return the parent Substance context of a Substance handle
    @param substanceHandle Pointer to the Substance handle.
    @return Returns the context used at handle creation or NULL if the handle
		is not valid. */
SUBSTANCE_EXPORT SubstanceContext* substanceHandleGetParentContext(
	SubstanceHandle* substanceHandle);


/** @brief Transfer internal cache from sourceHandle to destinationHandle
	@param destinationHandle Pointer to the Substance handle that grab internal
		cache items. This handle is initialized with the updated SBSBIN.
	@param sourceHandle Pointer to the Substance handle that contains internal
	    cache items to grab. This is the previous handle initialized with the
		previously linked SBSBIN.
	@param cacheMappingDataPtr Pointer on the cache mapping data generated by
		the Substance Linker when the updated SBSIN (destinationHandle) was
		linked (with cache mapping of the previous SBSBIN as previousCacheData).
		See documentation of substanceLinkerHandleGetCacheMapping in
		./linker/handle.h file for further information.
	@pre cacheMappingDataPtr must contains the cache mapping data corresponding
		to the delta of previous (sourceHandle) and updated SBSBIN
		(destinationHandle).
	@pre There must be no generation processes currently running both in
		sourceHandle and destinationHandle.
	@warning No commands should pending in destinationHandle (push operations
		not yet processed by a substanceHandleStart call).
	@warning The current hardware resources state is NOT transfered.
	@post Internal cache items that are still coherent are transfered from
		sourceHandle to destinationHandle. Coherent inputs are correctly set.
	@return Returns 0 if succeeded, an error code otherwise.

	Allows to switch from a previously linked SBSBIN to an updated SBSBIN
	without loosing internal cache items. */
SUBSTANCE_EXPORT unsigned int substanceHandleTransferCache(
	SubstanceHandle* destinationHandle,
    SubstanceHandle* sourceHandle,
	const unsigned char* cacheMappingDataPtr);


/** @brief Set the user data associated to the handle
	@param substanceHandle Pointer to the Substance handle to set data.
	@param userData User data (integer/pointer) to associate with the handle.
		This value can be grabbed in callback functions (by using
		substanceHandleGetUserData). User data is provided for convenience
		only, this value is not interpreted by the engine.
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceHandleSetUserData(
	SubstanceHandle* substanceHandle,
	size_t userData);

/** @brief Get the user data associated to the handle
    @param substanceHandle Pointer to the Substance handle to get data.
    @param[out] userData Sets the User data associated with the handle w/
		substanceHandleSetUserData (user data default value is 0).
    @return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceHandleGetUserData(
	SubstanceHandle* substanceHandle,
	size_t* userData);

/** @brief Set the maximum node size to be used during rendering
    @param substanceHandle Pointer to the Substance handle
    @param[in] maxNodeSizeWidthLog2  Maximum node width  (log2)
    @param[in] maxNodeSizeHeightLog2 Maximum node height (log2)
    @note This can be used to limit the size of internal nodes
          during rendering in order to limit the runtime cost,
          when rendering material previews for instance.
    @note Available since API version 1.9
    @return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceHandleSetMaxNodeSize(
	SubstanceHandle* substanceHandle,
	unsigned int     maxNodeSizeWidthLog2,
	unsigned int     maxNodeSizeHeightLog2);


/** @brief Internal use only */
SUBSTANCE_EXPORT unsigned int substanceHandleGetOutputs(
	SubstanceHandle* substanceHandle,
	unsigned int flags,
	unsigned int outIndexOrId,
	unsigned int outCount,
	void* result);


#endif /* ifndef _SUBSTANCE_HANDLE_H */
