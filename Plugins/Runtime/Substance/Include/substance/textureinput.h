/** @file textureinput.h
	@brief Texture used as input, contains platform specific texture info.
	@author Christophe Soum - Allegorithmic (christophe.soum@allegorithmic.com)
	@note This file is part of the Substance engine headers
	@date 20080212
	@copyright Allegorithmic. All rights reserved.

	Defines the SubstanceTextureInput structure. Contains the platform dependent
	texture definition (SubstanceTexture) and information about the format, size,
	etc. Used to parametrize Substance renders with a texture input. */

#ifndef _SUBSTANCE_TEXTUREINPUT_H
#define _SUBSTANCE_TEXTUREINPUT_H


/** Platform specific texture structure */
#include "texture.h"

/** Pixel formats definitions include */
#include "pixelformat.h"


/** @brief Substance engine input texture structure

	Contains platform specific texture and information about the format. */
typedef struct SubstanceTextureInput_
{
	/** @brief Platform specific texture structure */
	SubstanceTexture mTexture;
	
	/** @brief Width of the texture 
		Ignored if pixelFormat is Substance_PF_JPEG (retrieved from header) */
	unsigned int level0Width;
	
	/** @brief Height of the texture
		Ignored if pixelFormat is Substance_PF_JPEG (retrieved from header) */
	unsigned int level0Height;
	
	/** @brief Pixel format of the texture
		@see pixelformat.h */
	unsigned char pixelFormat;
	
	/** @brief Depth of the mipmaps (0: full pyramid, 1: only level0, etc.) */
	unsigned char mipmapCount;
	
} SubstanceTextureInput;	


#endif /* ifndef _SUBSTANCE_TEXTUREINPUT_H */
