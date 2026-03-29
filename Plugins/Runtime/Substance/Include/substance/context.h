/** @file context.h
	@brief Substance engine context structure and related functions definitions
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080108
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceContext structure. Initialized from platform specific
	informations. This structure is necessary to create Substance handles.
	The context can be used to fill in function callbacks.

	Summary of functions defined in this file:

	Context creation and deletion:
	  - substanceContextInit (macro that calls substanceContextInitImpl)
	  - substanceContextRelease

	Callback setting:
	  - substanceContextSetCallback

	States management:
	  - substanceContextRestoreStates (advanced usage)

	Resource free:
	  - substanceContextMemoryFree
	  - substanceContextTextureRelease
*/

#ifndef _SUBSTANCE_CONTEXT_H
#define _SUBSTANCE_CONTEXT_H



/** Platform specific API device structure */
#include "device.h"

/** Callback signature definitions */
#include "callbacks.h"

/** Platform specific texture structure */
#include "texture.h"

/** Platform dependent definitions */
#include "platformdep.h"

/** Error codes list */
#include "errors.h"

/** Version defines (used for API version) */
#include "version.h"


/** @brief Substance engine context structure pre-declaration.

	Must be initialized using the substanceContextInitImpl function and released
	using the substanceContextRelease function. */
struct SubstanceContext_;

/** @brief Substance engine handle structure pre-declaration (C/C++ compatible). */
typedef struct SubstanceContext_ SubstanceContext;



/** @brief Substance engine context initialization from platform specific data
	@param[out] substanceContext Pointer to the Substance context to initialize.
	@param platformSpecificDevice Platform specific structure used to initialize
		the context.
	@post The context is now valid (if no error occurred).
	@return Returns 0 if succeeded, an error code otherwise.

	This macro is used to call substanceContextInitImpl with correct arguments
	(Caller API version and platform).

	See the substanceContextInitImpl documentation just below for futher
	information. */
#define substanceContextInit(substanceContext,platformSpecificDevice) \
	substanceContextInitImpl( \
		substanceContext, \
		platformSpecificDevice, \
		SUBSTANCE_API_VERSION, \
		(SubstanceEngineIDEnum)SUBSTANCE_API_PLATFORM)

/** @brief Substance engine context initialization from platform specific data
	@param[out] substanceContext Pointer to the Substance context to initialize.
	@param platformSpecificDevice Platform specific structure used to initialize
		the context. (cf. device.h). This parameter can be NULL in order to
		create a READ-ONLY context. A read-only context can be only used to get
		Substance data information.
	@param apiVersion The value of this parameter should be
		SUBSTANCE_API_VERSION. Ensures that the header files against which an
		application is compiled match the version of the engine dynamic
		libraries.
	@param apiPlatform The value of this parameter should be
		SUBSTANCE_API_PLATFORM. Ensures that the header files against which an
		application is compiled match the platform API of the engine dynamic
		libraries.
	@pre The context must be previously un-initialized or released.
	@pre The platformSpecificDevice structure, if provided, must be correctly
		filled
	@post The context is now valid (if no error occurred).
	@return Returns 0 if succeeded, an error code otherwise.

	This can use API/hardware related resources: in particular, this function
	initialize the default GPU API states required by the Substance Engine. For
	performances purpose, not all GPU states are initialized: please read the
	documentation specific to your platform implementation. If GPU API states
	are modified by the user between the context initialization and a following
	computation, the substanceContextRestoreStates function must be called
	(e.g. callbacks functions, time-slicing computation, tweaks change, etc.).
	If platformSpecificDevice is NULL, no hardware related functions are
	called.<BR>
	Only one context can be created at a time on a given device.

	A helper macro 'substanceContextInit' is provided. It calls this function
	with apiVersion and apiPlatform arguments correctly filled. */
SUBSTANCE_EXPORT unsigned int substanceContextInitImpl(
	SubstanceContext** substanceContext,
	SubstanceDevice* platformSpecificDevice,
	unsigned int apiVersion,
	SubstanceEngineIDEnum apiPlatform);

/** @brief Substance engine context release
	@param substanceContext Pointer to the Substance context to release.
	@pre The context must be previously initialized and every handle that uses
	this context must be previously released.
	@post This context is not valid any more (it can be re-initialized).
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceContextRelease(
	SubstanceContext* substanceContext);

/** @brief Set Substance engine context callback
	@param substanceContext Pointer to the Substance context to set the callback for.
	@param callbackType Type of the callback to set. See the SubstanceCallbackEnum
	enumeration in callback.h.
	@param callbackPtr Pointer to the callback function to set.
	@pre The context must be previously initialized.
	@pre The callback function signature and callback type must match
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceContextSetCallback(
	SubstanceContext* substanceContext,
	SubstanceCallbackEnum callbackType,
	void *callbackPtr);

/** @brief Memory de-allocation function ("free") used by Substance
	@param substanceContext Pointer to a valid Substance context
	@param bufferPtr The buffer to free.
	@pre The context must be previously initialized.
	@pre The buffer to free must have been allocated by Substance.
	@return Returns 0 if succeeded, an error code otherwise.

	On BLEND platform, this function must be used to delete buffers allocated by
	the substance engine (output texture buffers).
	Useful only if the user "malloc" callback is not defined. */
SUBSTANCE_EXPORT unsigned int substanceContextMemoryFree(
	SubstanceContext* substanceContext,
	void* bufferPtr);

/** @brief Texture output result generated by Substance release function
	@param substanceContext Pointer to a valid Substance context
	@param resultTexture The texture generated by Substance to release.
	@pre The context must be previously initialized.
	@pre The texture to release must have been generated by Substance.
	@return Returns 0 if succeeded, an error code otherwise.

	This function can be used to release textures allocated by the substance
	engine (output texture). Equivalent to substanceContextMemoryFree on texture
	buffer on BLEND platform. */
SUBSTANCE_EXPORT unsigned int substanceContextTextureRelease(
	SubstanceContext* substanceContext,
	SubstanceTexture* resultTexture);

/** @brief Restore initial platform dependent states
	@param substanceContext Pointer to the Substance context
	@pre The context must be previously initialized and not 'read-only' (see
		substanceContextInit for further information).
	@return Returns 0 if succeeded, an error code otherwise.

	If GPU API states are modified by the user after the context initialization
	(e.g. the game scene is rendered), this function must be called just before
	the following computation (e.g. before substanceHandleStart call or at the
	end of a callback function).<BR>
	Partially restore the GPU API state as when the context was created. Not all
	GPU states are initialized: please read the documentation specific to your
	platform implementation. */
SUBSTANCE_EXPORT unsigned int substanceContextRestoreStates(
	SubstanceContext* substanceContext);

/** @brief  Return an opaque handle to the graphics device for tooling purposes (internal use only)
	@param  substanceContext Pointer to the Substance context
	@pre    The context must be previously initialized.
	@return Returns an opaque handle if succeeded, NULL otherwise.
*/
SUBSTANCE_EXPORT void* substanceContextGetOpaqueGraphicsDevice(
	SubstanceContext* substanceContext);

#endif /* ifndef _SUBSTANCE_CONTEXT_H */
