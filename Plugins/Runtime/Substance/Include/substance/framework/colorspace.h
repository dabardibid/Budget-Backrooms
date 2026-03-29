//! @file colorspace.h
//! @brief Substance Air color space tools
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_COLORSPACE_H
#define _SUBSTANCE_AIR_FRAMEWORK_COLORSPACE_H

#include "channel.h"

namespace SubstanceAir
{

//! @brief Enumeration of color space interpretation
//! Used as argument of each external resources output
enum ColorSpace
{
	//! @brief Interpreted as Linear color space image
	//! @note Equivalent to Passthru, interpreted as-is.
	ColorSpace_Linear,

	//! @brief Interpreted as sRGB color space image
	//! @warning L8 format stored as sRGB8, sL8 is not supported.
	ColorSpace_sRGB,

	//! @brief Interpreted as raw values, NOT color/luminance space.
	//! Passthru of normalized/float values.
	ColorSpace_Passthru,

	//! @brief Interpreted as signed normalized raw values.
	//! Signed -1..1 values stored as 0..1 normalized values. 
	ColorSpace_SNorm,

	//! @brief Interpreted as XYZ Direct normal stored in RGB channel.
	//! Right handed syscoord: OpenGL
	ColorSpace_NormalXYZRight,

	//! @brief Interpreted as XYZ Indirect normal stored in RGB channel.
	//! Left handed syscoord: Direct3D
	ColorSpace_NormalXYZLeft,

	ColorSpace_COUNT
};

//! @brief Choose color space automatically for the given channel and format.
//! @param channel the ChannelUse enum
//! @param isFloatFormat must be false for 8 or 16 bits integer formats, true otherwise.
//!
//!  * Diffuse, Emissive, Specular        
//!       -> Integer formats : sRGB
//!       -> Float formats   : Linear
//!  * Height, Displacement    
//!       -> Integer formats : SNorm
//!       -> Float formats   : Passthru
//!  * Normal
//!       -> All formats     : NormalXYZLeft
//!  * <Others>
//!       -> All formats     : Passthru
ColorSpace getDefaultColorSpace(ChannelUse channel, bool isFloatFormat)
{
	switch (channel)
	{
		case Channel_BaseColor:
		case Channel_Emissive:
		case Channel_Specular:
		case Channel_Transmissive:
		case Channel_Diffuse:
		case Channel_CoatColor:
			return isFloatFormat ? ColorSpace_Linear : ColorSpace_sRGB;

		case Channel_Height:
		case Channel_Displacement:
			return isFloatFormat ? ColorSpace_Passthru : ColorSpace_SNorm;

		case Channel_Normal:
		case Channel_CoatNormal:
			return ColorSpace_NormalXYZLeft;

		case Channel_SpecularLevel:
		default:
			return ColorSpace_Passthru;
	}
}

} // namespace SubstanceAir


#endif //_SUBSTANCE_AIR_FRAMEWORK_COLORSPACE_H