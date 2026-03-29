/** @file context.h
	@brief Substance linker context structure and related functions definitions
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance linker headers
	@date 20100406
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceContext structure. This structure is necessary to 
	create Substance linker handles.
	The context can be used to fill in linking options/parameters and 
	function callbacks. 
	
	Summary of functions defined in this file:
	
	Context creation and deletion:
	  - substanceLinkerContextInit
	  - substanceLinkerContextRelease
	  
	Options setting:
	  - substanceLinkerContextSetOption

	Callback setting:
	  - substanceLinkerContextSetCallback
*/

#ifndef _SUBSTANCE_LINKER_CONTEXT_H
#define _SUBSTANCE_LINKER_CONTEXT_H


/** Linking options definitions */
#include "options.h"

/** Callback signature definitions */
#include "callbacks.h"

/** Version related definitions */
#include "version.h"

/** Platform dependent definitions */
#include "platformdep.h"

/** Error codes list */
#include "errors.h"



/** @brief Substance linker context structure pre-declaration.

	Must be initialized using the substanceLinkerContextInit function and 
	released using the substanceLinkerContextRelease function. */
struct SubstanceLinkerContext_;

/** @brief Substance linker handle structure pre-declaration (C/C++ compatible). */
typedef struct SubstanceLinkerContext_ SubstanceLinkerContext;



/** @brief Substance linker context initialization
	@param[out] substanceLinkerContext Pointer to the Substance Linker context 
		to initialize.
	@param substanceLinkerApiVersion The value of this parameter should be 
		SUBSTANCE_LINKER_API_VERSION. Ensure that the header files against 
		which an application is compiled match the version of the linker 
		dynamic libraries.
	@param targetEngineId Numeric identifier of the targeted engine. See
		SubstanceEngineId enum in ../engineid.h header. Binaries produced with
		the context are dedicated to the engine identified with this parameter. 
	@pre The context must be previously un-initialized or released.
	@post The context is now valid (if no error occurred).
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerContextInit(
	SubstanceLinkerContext** substanceLinkerContext,
	unsigned int substanceLinkerApiVersion,
	unsigned int targetEngineId);

/** @brief Substance linker context release
	@param substanceLinkerContext Pointer to the Substance context to release.
	@pre The context must be previously initialized and every handle that uses
	this context must be previously released.
	@post This context is not valid any more (it can be re-initialized).
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerContextRelease(
	SubstanceLinkerContext* substanceLinkerContext);


/** @brief Set linking option/parameters
	@param substanceLinkerContext Pointer to the context to set the option.
	@param optionType Type of the option to set. See the 
		SubstanceLinkerOptionEnum enumeration in options.h.
	@param value Option value (union).
	@pre The context must be previously initialized.
	@pre The value must have the required option type.
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerContextSetOption(
	SubstanceLinkerContext* substanceLinkerContext,
	SubstanceLinkerOptionEnum optionType,
	SubstanceLinkerOptionValue value);

/** @brief Set Substance linker context callback
	@param substanceLinkerContext Pointer to the context to set the callback for.
	@param callbackType Type of the callback to set. See the 
		SubstanceLinkerCallbackEnum enumeration in callback.h.
	@param callbackPtr Pointer to the callback function to set.
	@pre The context must be previously initialized.
	@pre The callback function signature and callback type must match
	@return Returns 0 if succeeded, an error code otherwise. */
SUBSTANCE_EXPORT unsigned int substanceLinkerContextSetCallback(
	SubstanceLinkerContext* substanceLinkerContext,
	SubstanceLinkerCallbackEnum callbackType,
	void *callbackPtr);
	
	
#endif /* ifndef _SUBSTANCE_LINKER_CONTEXT_H */
