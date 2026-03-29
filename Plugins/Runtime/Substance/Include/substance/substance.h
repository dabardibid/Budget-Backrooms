/** @file substance.h
	@brief Header for the Substance engine API. Manages the texture generation.
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080107
	@copyright Allegorithmic. All rights reserved.

	Texture generation HOWTO:
	  - 1. Initialize a SubstanceContext (Platform dependent).
	  - 2. Create a SubstanceHandle from the SubstanceContext and a buffer that
	  contains the Substance cooked data to render.
	  - 3. Set up the generation parameters, select the textures to render, fill
	  in the desired function callbacks, and fill in the inputs.
	  - 4. Launch the generation process (synchronous or asynchronous).
	  - 5. Grab the generated textures progressively (via a callback
	  function) or at a stretch at the end of generation.
	  - 6. Release the handle.
	  - 7. Release the context.
*/

/** @mainpage Substance - Engine API Documentation

	@section intro_sec Introduction

	The Substance Engine API allows the generation of textures produced by
	the Substance authoring tools. The main features are:
	  - It allows synchronous, time-slicing or asynchronous generation. 
	  - Both streaming or at a stretch strategies can be used and combined. 
	  - This API is platform independant and can be used by C or C++ 
	  applications.
	  - Hardware resources (CPU/GPU, memories) utilization can be precisely 
	    controled.

	@section platforms_sec Platforms
	
	Set the <B>SUBSTANCE_PLATFORM_<I>xxx</I></B> preprocessor macro 
	corresponding to the required platform 
	(see platformdep.h for the list of platforms).
	
	The platform <B>SUBSTANCE_PLATFORM_BLEND</B> is a specific platform fully 
	independent to the actual platform and the actual engine implementation: 
	the texture results are stored in the system memory.

	@section start_sec Getting started with substance
 
	The include entry point is substance.h header. 
	It contains also a texture generation HOWTO.
	
	The most important header file is handle.h one. It contains Substance Engine
	handle structure and related functions definitions. This handle is used to 
	set up generation parameters, select textures to render, fill-in inputs, 
	launch the generation process and grab resulting textures.

 */


#ifndef _SUBSTANCE_SUBSTANCE_H
#define _SUBSTANCE_SUBSTANCE_H

/** Handle structure and related functions definitions */
#include "handle.h"

/** Context structure and related functions definitions */
#include "context.h"

/** Platform specific texture (results) structure */
#include "texture.h"

/** Texture used as input, contains platform specific texture and format info */
#include "textureinput.h"

/** Platform specific API device structure */
#include "device.h"

/** Callback signature definitions */
#include "callbacks.h"

/** Platform dependent definitions */
#include "platformdep.h"

/** Error codes list include */
#include "errors.h"

/** Current engine version accessor defines */
#include "version.h"

#endif /* ifndef _SUBSTANCE_SUBSTANCE_H */
