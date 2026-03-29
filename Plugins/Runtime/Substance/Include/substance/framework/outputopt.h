//! @file outputopt.h
//! @brief Substance Air definition of output options used at package creation
//! @author Christophe Soum - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_OUTPUTOPT_H
#define _SUBSTANCE_AIR_FRAMEWORK_OUTPUTOPT_H

#include "typedefs.h"


namespace SubstanceAir
{

//! @brief Enumeration of all allowed output formats
//! These flags are used in OutputOptions::mAllowedFormats
enum AllowedFormat
{
	Format_None            = 0x000, //!< Cannot be used, at least one format must be allowed

	Format_RGBA8           = 0x001, //!< Color RGBA 8 bits per components (32bpp)
	Format_L8              = 0x002, //!< Luminance 8 bits
	Format_RGBA16          = 0x004, //!< Color RGBA 16 bits per components (64bpp)
	Format_L16             = 0x008, //!< Luminance 16 bits
	Format_RGBA16F         = 0x80,  //!< Color RGBA Half-float (64bpp)
	Format_L16F            = 0x100, //!< Luminance Half-float (16 bits)
	Format_RGBA32F         = 0x200, //!< Color RGBA floating point (128bpp)
	Format_L32F            = 0x400, //!< Luminance floating point (32 bits)

	Format_BC1             = 0x010, //!< BC1 (DXT1) compressed format
	Format_BC2             = 0x020, //!< BC2 (DXT2/DXT3) compressed format
	Format_BC3             = 0x040, //!< BC3 (DXT4/DXT5) compressed format
	Format_BC4             = 0x080, //!< BC4 (ATI1/RGTC1) compressed format
	Format_BC5             = 0x100, //!< BC5 (ATI2/RGTC2) compressed format
	
	/* Helpers: */
	
	Format_Mask_Raw8bpc    = 0x3,   //!< L8,RGBA8
	Format_Mask_Raw16I     = 0xC,   //!< L16,RGBA16
	Format_Mask_Raw16F     = 0x180, //!< L16F,RGBA16F
	Format_Mask_Raw32F     = 0x600, //!< L32F,RGBA32F
	Format_Mask_RawInteger = 0xF,   //!< L8,RGBA8,L16,RGBA16
	Format_Mask_RawFloat   = 0x780, //!< L16F,RGBA16F,L32F,RGBA32F
	Format_Mask_Raw        = 0x78F, //!< L8,RGBA8,L16x,RGBA16x,L32F,RGBA32F
	Format_Mask_Compressed = 0x70,  //!< BC1,BC2,BC3
	Format_Mask_ALL        = 0x7FF  //!< All formats allowed
};


//! @brief Enumeration of mipmap override options
//! This enumeration is used in OutputOptions::mMipmap
enum MipmapOption
{
	Mipmap_Default,     //!< Keep original (set at authoring time) mipmap option
	Mipmap_ForceFull,   //!< Force full mipmap pyramid
	Mipmap_ForceNone    //!< Force no mipmaps (only base level)
};


//! @brief Description of output format override for a whole package
//!
//! Substance assets have default output options (format, mipmap, etc.) defined
//! at authoring time. This structure allows you to override these options for
//! a whole package. See OutputInstance::overrideFormat() for per-output format
//! override.
struct OutputOptions
{
	//! @brief Set of allowed formats, combination of AllowedFormat enums
	//! Default value: Format_Mask_ALL, all formats are allowed.
	//!
	//! Not allowed formats are replaced by allowed ones for the best.
	//! This simplifies integration process: e.g. for applications that do not
	//! support compressed (DXT) or e.g. 16F formats.  
	unsigned int mAllowedFormats;

	//! @brief Mipmap override option, allows to force no/full mipmaps
	//! Default value: Mipmap_Default, keep original mipmap option.
	MipmapOption mMipmap;
	
	
	//! @brief Default constructor
	OutputOptions() :
		mAllowedFormats(Format_Mask_ALL),
		mMipmap(Mipmap_Default)
	{
	}
};


} // namespace SubstanceAir


#endif //_SUBSTANCE_AIR_FRAMEWORK_OUTPUTOPT_H
