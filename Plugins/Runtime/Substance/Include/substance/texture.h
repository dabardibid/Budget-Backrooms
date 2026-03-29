/** @file texture.h
	@brief Platform specific texture (results) structure
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080108
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceTexture structure. Platform dependent definition. Used
	to return resulting output textures. */

#ifndef _SUBSTANCE_TEXTURE_H
#define _SUBSTANCE_TEXTURE_H


/** Platform dependent definitions */
#include "platformdep.h"


#if defined(SUBSTANCE_PLATFORM_OGL3)

	/** @brief Substance engine texture (results) structure

	    OpenGL version. */
	typedef struct SubstanceTexture_
	{
		/** @brief OpenGL texture name */
		unsigned int textureName;

	} SubstanceTexture;

#elif defined(SUBSTANCE_PLATFORM_D3D10PC)

	/** @brief Substance engine texture (results) structure

	    Direct3D10 PC version. */
	typedef struct SubstanceTexture_
	{
		/** @brief Direct3D Texture handle */
		ID3D10Texture2D* handle;

		/** @brief Format override if the texture is typeless (only used for inputs) */
		DXGI_FORMAT format;

	} SubstanceTexture;

#elif defined(SUBSTANCE_PLATFORM_D3D11PC) || \
      defined(SUBSTANCE_PLATFORM_D3D11XBOXONE)

	/** @brief Substance engine texture (results) structure

	    Direct3D11 PC/XboxOne versions. */
	typedef struct SubstanceTexture_
	{
		/** @brief Direct3D Texture handle */
		ID3D11Texture2D* handle;

		/** @brief Format override if the texture is typeless (only used for inputs) */
		DXGI_FORMAT format;

	} SubstanceTexture;

#elif defined(SUBSTANCE_PLATFORM_D3D12XBOXONE)

	/** @brief Substance engine texture (results) structure

	    Direct3D12 XboxOne version. */
	typedef struct SubstanceTexture_
	{
		/** @brief Direct3D Resource handle */
		ID3D12Resource* handle;

		/** @brief Format override if the texture is typeless (only used for inputs) */
		DXGI_FORMAT format;

	} SubstanceTexture;

#elif defined(SUBSTANCE_PLATFORM_PS4)

	/** @brief Substance engine texture (results) structure

	    PS4 GNM version. */

	/** @brief Helper structure used to keep storage-relatd information */
	typedef struct GnmStorageInfo {
		void*  mPtr;    /* Mapped pointer, visible from CPU and GPU */
		off_t  mOffset; /* Physical offset, only valid if memory allocation was NOT performed via the callback */
		size_t mLength; /* Size (in bytes), might be different from the requested size because of API constraints */
	} GnmStorageInfo;

	typedef struct SubstanceTexture_
	{
		/** @brief GNM texture description. */
		sce::Gnm::Texture gnmTexture;

		/** @brief Storage-related info, needed for memory release purposes
		           Set to NULL for texture inputs, and do not modify for regular output textures */
		GnmStorageInfo storageInfo;

	} SubstanceTexture;

#elif defined(SUBSTANCE_PLATFORM_VULKAN)

	/** @brief Substance engine texture (results) structure

		Vulkan version.
		
		@warning Texture OUTPUT must be deleted by substanceContextTextureRelease() */
	typedef struct SubstanceTexture_
	{
		/** @brief Vulkan image, 
			If used as INPUT: must be ready for pixel read, pixel read ready
				again after use.
			If used as OUTPUT: ready for pixel read

			Stage: VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			Access: VK_ACCESS_SHADER_READ_BIT,
			Layout: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			
			@warning Handle OUTPUT must be deleted by substanceContextTextureRelease() */
		VkImage image;

		/** @brief Internal texture identifier
			Only valid if OUTPUT. Used for texture deletion by substanceContextTextureRelease() */
		uint32_t internalId;

	} SubstanceTexture;

#elif defined(SUBSTANCE_PLATFORM_SAL)

	/** @brief Substance engine texture (results) structure

		SAL version. */
	typedef struct SubstanceTexture_
	{
		/** @brief SAL texture handle, 
		If used as INPUT: access must be SREBufferAccessType_PixelShaderRead,
		pixel read ready again after use.
		If used as OUTPUT: access is SREBufferAccessType_PixelShaderRead */
		SRETexture handle;

	} SubstanceTexture;

#elif defined(SUBSTANCE_PLATFORM_BLEND)

	/** @brief Substance engine texture (results) structure

	    Blend platform. Result in system memory. */
	typedef struct SubstanceTexture_
	{
		/** @brief Pointer to the result array

			This value is NULL if the buffer of this output has been allocated
			by the SubstanceCallbackInPlaceOutput callback. See callbacks.h for
			further information.

			If present, the buffer contains all the mipmap levels concatenated
			without any padding. If a different memory layout is required, use
			the SubstanceCallbackInPlaceOutput callback. */
		void *buffer;

		/** @brief Width of the texture base level */
		unsigned short level0Width;

		/** @brief Height of the texture base level */
		unsigned short level0Height;

		/** @brief Pixel format of the texture
			@see pixelformat.h */
		unsigned char pixelFormat;

		/** @brief Channels order in memory

			One of the SubstanceChannelsOrder enum (defined in pixelformat.h).

			The default channel order  depends on the format and on the
			Substance engine concrete implementation (CPU/GPU, GPU API, etc.).
			See the platform specific documentation. */
		unsigned char channelsOrder;

		/** @brief Depth of the mipmap pyramid: number of levels */
		unsigned char mipmapCount;

	} SubstanceTexture;

#else

	/** @todo Add other APIs texture definition */
	#error NYI

#endif



#endif /* ifndef _SUBSTANCE_TEXTURE_H */
