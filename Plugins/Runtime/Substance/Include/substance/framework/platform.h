//! @file platform.h
//! @brief Substance engine platform defines
//! @author Christophe Soum - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Defines used to handle multiple Substance Engine platform APIs.
//! The AIR_PLATFORM_AGNOSTIC define must be set before including any framework
//! header in each scope where the platform is not yet determined. Otherwise,
//!	the BLEND (textures in system memory) platform is used by default. To use
//!	an other platform, define the corresponding SUBSTANCE_PLATFORM_xxx macro.
//!
//! @warning The Substance engine platform is different from engine
//!		implementation, e.g. a Direct3D10 BLEND Engine uses internally D3D10
//!		to render textures but exposes results as buffers in system memory.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_PLATFORM_H
#define _SUBSTANCE_AIR_FRAMEWORK_PLATFORM_H


#include "typedefs.h"

#include <substance/engineid.h>

#include <assert.h>
#include <memory.h>


namespace SubstanceAir
{


//! @brief Generic texture holder agnostic from concrete platform
struct TextureAgnostic
{
	//! @brief Union-like data that can hold any concrete SubstanceTexture
	//! Large enough to hold any SubstanceTexture, void* is used to force
	//! pointer alignment.
	void* textureAny[4];

	//! @brief Concrete texture platform enumeration
	//! Used to check platform match when casting
	SubstanceEngineIDEnum platform;


	//! @brief Constructor, 0 filled union
	TextureAgnostic() : platform(Substance_EngineID_FORCEDWORD)
		{ memset(&textureAny,0,sizeof(textureAny)); }
};


//! @brief Definition of the platform agnostic texture input description
//! Extend agnostic texture holder.
struct TextureInputAgnostic
{
	//! @brief Union-like data that can hold any concrete SubstanceTextureInput
	//! Large enough to hold any SubstanceTextureInput, void* is used to force
	//! pointer alignment.
	void* textureInputAny[6];

	//! @brief Concrete texture platform enumeration
	//! Used to check platform match when casting
	SubstanceEngineIDEnum platform;


	//! @brief Default constructor, 0 filled texture input data
	TextureInputAgnostic() { memset(&textureInputAny,0,sizeof(textureInputAny)); }
};


//! @brief Int to type struct used to avoid function signature conflicts
template <int N>
struct IntToType { enum { Number = N }; };


} // namespace SubstanceAir

#if defined(AIR_PLATFORM_AGNOSTIC)

	// The Engine Substance platform depend on engine DLL currently used
	// Concrete platform don't known at this point.

#else  // if defined(AIR_PLATFORM_AGNOSTIC)

	// Platform is known at this scope

	#if   defined(SUBSTANCE_PLATFORM_D3D10PC)
	#elif defined(SUBSTANCE_PLATFORM_D3D11PC)
	#elif defined(SUBSTANCE_PLATFORM_D3D11XBOXONE)
	#elif defined(SUBSTANCE_PLATFORM_OGL3)
	#elif defined(SUBSTANCE_PLATFORM_PS4)
	#elif defined(SUBSTANCE_PLATFORM_VULKAN)
	#elif defined(SUBSTANCE_PLATFORM_SAL)
	#elif defined(SUBSTANCE_PLATFORM_BLEND)
	#else
		// Use BLEND Substance Engine platform if not specified AND not
		// platform agnostic scope
		#define SUBSTANCE_PLATFORM_BLEND
	#endif

	#include <substance/texture.h>
	#include <substance/textureinput.h>


	namespace SubstanceAir
	{


	//! @brief Dummy parameter used to create different function signature
	//! Avoid conflicts between modules using different platforms
	#define AIR_PLATFORM_DUMMYPARAM \
		IntToType<SUBSTANCE_API_PLATFORM>*_dummyParam=nullptr


	//! @brief Test if texture holder contains a compatible concrete texture
	//! @pre The platform match (isPlatformMatch(texture)==true)
	inline bool isPlatformMatch(
		const TextureAgnostic& texture,
		AIR_PLATFORM_DUMMYPARAM)
	{
		(void)_dummyParam;
		return texture.platform == SUBSTANCE_API_PLATFORM;
	}

	//! @brief Concrete texture cast function defined is platform is known
	//! @pre The platform match (isPlatformMatch(texture)==true)
	inline SubstanceTexture& castToConcrete(
		TextureAgnostic& texture,
		AIR_PLATFORM_DUMMYPARAM)
	{
		(void)_dummyParam;
		assert(isPlatformMatch(texture));
		assert(sizeof(SubstanceTexture)<=sizeof(texture.textureAny));
		return *reinterpret_cast<SubstanceTexture*>(texture.textureAny);
	}

	//! @brief Concrete texture cast function defined is platform is known
	//! Const overload
	//! @pre The platform match (isPlatformMatch(texture)==true)
	inline const SubstanceTexture& castToConcrete(
		const TextureAgnostic& texture,
		AIR_PLATFORM_DUMMYPARAM)
	{
		(void)_dummyParam;
		assert(isPlatformMatch(texture));
		assert(sizeof(SubstanceTexture)<=sizeof(texture.textureAny));
		return *reinterpret_cast<const SubstanceTexture*>(texture.textureAny);
	}


	//! @brief Test if texture holder contains a compatible concrete texture inp
	//! @pre The platform match (isPlatformMatch(texture)==true)
	inline bool isPlatformMatch(
		const TextureInputAgnostic& textureInput,
		AIR_PLATFORM_DUMMYPARAM)
	{
		(void)_dummyParam;
		return textureInput.platform == SUBSTANCE_API_PLATFORM;
	}

	//! @brief Concrete texture input cast function defined is platform is known
	//! @pre The platform match (isPlatformMatch(texture)==true)
	inline SubstanceTextureInput& castToConcrete(
		TextureInputAgnostic& textureInput,
		AIR_PLATFORM_DUMMYPARAM)
	{
		(void)_dummyParam;
		assert(isPlatformMatch(textureInput));
		assert(sizeof(SubstanceTextureInput)<=sizeof(textureInput.textureInputAny));
		return *reinterpret_cast<SubstanceTextureInput*>(textureInput.textureInputAny);
	}

	//! @brief Concrete texture input cast function defined is platform is known
	//! Const overload
	//! @pre The platform match (isPlatformMatch(texture)==true)
	inline const SubstanceTextureInput& castToConcrete(
		const TextureInputAgnostic& textureInput,
		AIR_PLATFORM_DUMMYPARAM)
	{
		(void)_dummyParam;
		assert(isPlatformMatch(textureInput));
		assert(sizeof(SubstanceTextureInput)<=sizeof(textureInput.textureInputAny));
		return *reinterpret_cast<const SubstanceTextureInput*>(textureInput.textureInputAny);
	}


	} // namespace SubstanceAir


#endif // if defined(AIR_PLATFORM_AGNOSTIC)


#endif //_SUBSTANCE_AIR_FRAMEWORK_PLATFORM_H
