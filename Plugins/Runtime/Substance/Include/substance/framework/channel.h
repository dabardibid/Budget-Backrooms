//! @file channel.h
//! @brief Substance Air channels enumeration definition
//! @author Christophe Soum - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_CHANNEL_H
#define _SUBSTANCE_AIR_FRAMEWORK_CHANNEL_H

namespace SubstanceAir
{

//! @brief The list of channels roles available in Designer
enum ChannelUse
{
	Channel_Diffuse,
	Channel_BaseColor,
	Channel_Opacity,
	Channel_Emissive,
	Channel_Ambient,
	Channel_AmbientOcclusion,
	Channel_Mask,
	Channel_Normal,
	Channel_Bump,
	Channel_Height,
	Channel_Displacement,
	Channel_Specular,
	Channel_SpecularLevel,
	Channel_SpecularColor,
	Channel_Glossiness,
	Channel_Roughness,
	Channel_AnisotropyLevel,
	Channel_AnisotropyAngle,
	Channel_Transmissive,
	Channel_Reflection,
	Channel_Refraction,
	Channel_CoatWeight,
	Channel_CoatSpecularLevel,
	Channel_CoatColor,
	Channel_CoatRoughness,
	Channel_CoatNormal,
	Channel_Environment,
	Channel_IOR,
	Channel_Scattering,
	/* The SCATTERINGx channels below are deprecated and replaced by the one above */
	Channel_SCATTERING0,
	Channel_SCATTERING1,
	Channel_SCATTERING2,
	Channel_SCATTERING3,
	Channel_Any,
	Channel_Metallic,
	Channel_Panorama,
	Channel_PhysicalSize,
	Channel_UNKNOWN,
	Channel_INTERNAL_COUNT
};


//! @brief Helper: get channel names corresponding to ChannelUse
//! @return Return array of Channel_INTERNAL_COUNT strings
const char*const* getChannelNames();


} // namespace SubstanceAir


#endif //_SUBSTANCE_AIR_FRAMEWORK_CHANNEL_H
