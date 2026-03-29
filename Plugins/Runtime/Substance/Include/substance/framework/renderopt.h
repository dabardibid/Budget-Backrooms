//! @file renderopt.h
//! @brief Substance Air definition of render options used by renderer
//! @author Christophe Soum - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_RENDEROPT_H
#define _SUBSTANCE_AIR_FRAMEWORK_RENDEROPT_H

#include "typedefs.h"


namespace SubstanceAir
{


//! @brief Substance render options
//! Used at renderer creation and by Renderer::setOptions method.
//! Allows to set memory consumption budget and CPU usage.
struct RenderOptions
{
	//! @brief Memory budget allowed to renderer, in bytes.
	//! The default value is set to 512MB.
	size_t mMemoryBudget;

	//! @brief Maximum number of CPU Cores allowed to renderer.
	//! All cores used by default.
	//! @note Only relevant w/ CPU based Engines.
	size_t mCoresCount;

	//! @brief Maximum size (log2) of internal renders
	//! This can be used to limit the size of the intermediate nodes
	//! computed by the engine, so that nodes whose width/height are
	//! higher than the threshold are computed at the threshold instead.
	//! @note This limit is in log2 format, i.e. setting it to 12 limits
	//!       the width/height to 2048. Default is 15 (32768).
	size_t mMaxIntermRenderSize;

	//! @brief Default constructor
	RenderOptions() :
		mMemoryBudget(512*1024*1024),       // 512MB
		mCoresCount(32),                    // All
		mMaxIntermRenderSize(15)            // 32768
	{
	}
};


} // namespace SubstanceAir


#endif //_SUBSTANCE_AIR_FRAMEWORK_RENDEROPT_H
