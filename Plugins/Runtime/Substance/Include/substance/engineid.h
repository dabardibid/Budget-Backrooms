/** @file engineid.h
	@brief Substance engine identifier enumeration definition.
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance headers
	@date 20100414
	@copyright Allegorithmic. All rights reserved.

	Used by Substance Linker to generate dedicated binaries and by engines
	version structure (see version.h). */

#ifndef _SUBSTANCE_ENGINEID_H
#define _SUBSTANCE_ENGINEID_H


/** Basic type defines */
#include <stddef.h>


/** @brief Substance engine ID enumeration */
typedef enum
{
	Substance_EngineID_INVALID      = 0x0,    /**< Unknown engine, do not use */
	Substance_EngineID_BLEND        = 0x0,    /**< BLEND platform API */

	Substance_EngineID_ogl3         = 0xC,    /**< OpenGL3 */
	Substance_EngineID_d3d10pc      = 0x7,    /**< Direct3D 10 Windows */
	Substance_EngineID_d3d11pc      = 0xB,    /**< Direct3D 11 Windows */

	Substance_EngineID_xb1d3d11     = 0xD,    /**< Xbox One GPU (Direct3D 11) engine */
	Substance_EngineID_xb1d3d12     = 0x1E,   /**< Xbox One GPU (Direct3D 12) engine */
	/* Deprecated EngineID, use xb1d3d11 instead */
	Substance_EngineID_d3d11xb1     = Substance_EngineID_xb1d3d11,

	Substance_EngineID_ps4gnm       = 0xE,    /**< PS4 GPU (GNM) engine */

	Substance_EngineID_stdc         = 0x10,   /**< Pure Software (at least) engine */
	Substance_EngineID_sse2         = 0x13,   /**< SSE2 (minimum) CPU engine */
	Substance_EngineID_neon         = 0x1B,   /**< NEON (minimum) CPU engine */

	Substance_EngineID_xb1avx       = 0x1C,   /**< Xbox One CPU engine (AVX) */
	/* Deprecated EngineID, use xb1avx instead */
	Substance_EngineID_xb1          = Substance_EngineID_xb1avx,
	Substance_EngineID_ps4avx       = 0x1D,   /**< PS4 CPU engine (AVX) */

	Substance_EngineID_salvk        = 0x12,   /**< SAL Vulkan */
	Substance_EngineID_vulkan       = 0x14,   /**< Vulkan */
	Substance_EngineID_SAL          = 0x15,   /**< SAL abstraction layer API */

	/* etc. */

	Substance_EngineID_FORCEDWORD   = 0xFFFFFFFF /**< Force DWORD enumeration */

} SubstanceEngineIDEnum;


#endif /* ifndef _SUBSTANCE_ENGINEID_H */
