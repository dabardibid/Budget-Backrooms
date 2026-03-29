//! @file outputfmt.h
//! @brief Substance Air definition of structures used to overload output format
//! @author Christophe Soum - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_OUTPUTFMT_H
#define _SUBSTANCE_AIR_FRAMEWORK_OUTPUTFMT_H

#include "typedefs.h"


namespace SubstanceAir
{


//! @brief Structure used for output format/layout override
//! Allows to override pixel format, mipmap pyramid depth, horizontal and
//! vertical flipping, resize, per component value remapping (e.g. invert),
//! components shuffling and composition from other graph outputs (e.g. allows
//! channels composition to match shader layout).
struct OutputFormat
{
	enum Constant 
	{
		UseDefault        = ~0u, //!< Default value wild card
		ComponentEmpty    = 0,   //!< Used by Component::outputUid
		ComponentsCount   = 4,   //!< Maximum number of components per output
		MipmapFullPyramid = 0,   //!< Used by mipmapLevelsCount
		MipmapNone        = 1    //!< Used by mipmapLevelsCount
	};

	//! @brief Horizontal and vertical flip enumeration
	//! Used by hvFlip
	enum HVFlip
	{

		HVFlip_No         = 0x0, //!< No flipping (default value)
		HVFlip_Horizontal = 0x1, //!< Horizontal flip
		HVFlip_Vertical   = 0x2, //!< Vertical flip
		HVFlip_Both       = 0x3  //!< Horizontal and Vertical flip
	};

	//! @brief Per component make-up
	//! Allows component shuffling and composition from other
	//! outputs (of the same graph). Allows also values remapping (e.g. invert). 
	struct Component
	{
		//! @brief UID of the Output to use as source for this component
		//! Allows components shuffling across different outputs of the same
		//!	graph instance. Possible values: 
		//! 	* UseDefault: current output (default value).
		//! 	* ComponentEmpty: fill component with 0.
		//! 	* <other value>: Must be an Output UID of the same parent graph. 
		unsigned int outputUid;

		//! @brief Shuffling: source component index
		//!		0: Red
		//!		1: Green
		//!		2: Blue
		//!		3: Alpha channel
		unsigned int shuffleIndex;
	
		//! @brief Component value remapping: new minimum level (default: 0.0)
		float levelMin;
	
		//! @brief Component value remapping: new maximum level (default: 1.0) 
		float levelMax;


		//! @brief Default constructor, identity mapping
		Component();

		//! @brief Equality operator
		bool operator==(const Component& b) const;

		//! @brief Returns if identity (w/ component index)
		bool isIdentity(unsigned int index) const;
	};


	//! @brief Pixel format (or UseDefault to keep default one)
	//! Combination of SubstancePixelFormat enums (see substance/pixelformat.h
	//! for the complete list). Set to UseDefault to use default original 
	//! format (default value).
	unsigned int format;
	
	//! Mipmap pyramid depth
	//! 	* UseDefault: Default original mipmap pyramid depth (default value).
	//! 	* MipmapFullPyramid: full mipmap pyramid.
	//! 	* MipmapNone: Only one level, no mipmaps.
	//!     * <Other values>: explicit pyramid depth.
	unsigned int mipmapLevelsCount;
		
	//! @brief Horizontal and vertical flipping: one of HVFlip enumeration
	HVFlip hvFlip;
	
	//! @brief Width override of the output (set to UseDefault for default) 
	//! The upper power of 2 value is actually used. Either both width & height
	//! should be overridden or none.
	unsigned int forceWidth;
		
	//! @brief Height override of the output (set to UseDefault for default) 
	//! The upper power of 2 value is actually used. Either both height & width 
	//! should be overridden or none.
	unsigned int forceHeight;
	
	//! @brief Per component make-up: component shuffling and remapping
	//! By default: no shuffling and no remapping.
	//! Only the first item is used if grayscale format.
	Component perComponent[ComponentsCount];


	//! @brief Default constructor, identity format overload
	OutputFormat();

	//! @brief Equality operator
	bool operator==(const OutputFormat& b) const;
};



inline OutputFormat::Component::Component() :
	outputUid(static_cast<unsigned int>(UseDefault)),
	levelMin(0.f),
	levelMax(1.f)
{
}

inline bool OutputFormat::Component::isIdentity(unsigned int index) const
{
	return outputUid==UseDefault &&
		levelMin==0.f &&
		levelMax==1.f &&
		shuffleIndex==index;
}

inline bool OutputFormat::Component::operator==(const Component& b) const
{
	return outputUid==b.outputUid && shuffleIndex==b.shuffleIndex && 
		levelMin==b.levelMin && levelMax==b.levelMax;
}

inline OutputFormat::OutputFormat() :
	format(static_cast<unsigned int>(UseDefault)),
	mipmapLevelsCount(static_cast<unsigned int>(UseDefault)),
	hvFlip(HVFlip_No),
	forceWidth(static_cast<unsigned int>(UseDefault)),
	forceHeight(static_cast<unsigned int>(UseDefault))
{
	perComponent[0].shuffleIndex = 0;
	perComponent[1].shuffleIndex = 1;
	perComponent[2].shuffleIndex = 2;
	perComponent[3].shuffleIndex = 3;
}

inline bool OutputFormat::operator==(const OutputFormat& b) const 
{
	return format==b.format &&
		mipmapLevelsCount==b.mipmapLevelsCount &&
		hvFlip==b.hvFlip &&
		forceWidth==b.forceWidth &&
		forceHeight==b.forceHeight &&
		perComponent[0]==b.perComponent[0] &&
		perComponent[1]==b.perComponent[1] &&
		perComponent[2]==b.perComponent[2] &&
		perComponent[3]==b.perComponent[3];
}


} // namespace SubstanceAir


#endif //_SUBSTANCE_AIR_FRAMEWORK_OUTPUTFMT_H
