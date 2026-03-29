//! @file preset.h
//! @brief Substance Air preset definitions
//! @author Christophe Soum - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_PRESET_H
#define _SUBSTANCE_AIR_FRAMEWORK_PRESET_H

#include "typedefs.h"

#include <substance/inputdesc.h>



#include <iostream>


namespace SubstanceAir
{

class GraphInstance;


//! @brief Class that represents a preset of inputs
//! Presets can be loaded from a preset file (.SBSPRS) or can be embedded in a
//! graph description (GraphDesc::getPresets())
struct Preset
{
	//! @brief One input value structure
	struct InputValue
	{
		//! @brief 32 bits unsigned Unique Identifier
		//! Used for exact match.
		UInt mUid;

		//! @brief User-readable identifier of the input
		//! Used for matching a preset created from a graph to an another one.
		string mIdentifier;

		//! @brief Type of the input
		//! Used to check values types consistency.
		//! @see SubstanceIOType enum in substance/iotype.h
		SubstanceIOType mType;

		//! @brief Value/filepath of the input, stored as string
		string mValue;
	};

	//! @brief Container of input values
	typedef vector<InputValue> InputValues;

	//! @brief Apply mode, used by apply()
	enum ApplyMode
	{
		Apply_Reset,  //!< Graph inputs not listed in input values are reset
		Apply_Merge   //!< Non concerned graph inputs are not reset
	};


	//! @brief URL of the graph corresponding to this preset
	//! A preset can be applied to any graph, only input values that match per
	//!	identifier and per type are effectively set.
	string mPackageUrl;

	//! @brief Label of this preset (UI) UTF8 format
	string mLabel;

	//! @brief Description of this preset (UI) UTF8 format
	string mDescription;

	//! @brief List of preset inputs
	//! Only non default input values are recorded.
	InputValues mInputValues;


	//! @brief Apply this preset to a graph instance
	//! @param graph The target graph instance to apply this preset
	//! @param mode Reset to default other inputs of merge w/ previous values
	//! @note In order to load input image defined in the preset, the
	//!		GlobalCallbacks::loadInputImage callback must be defined.
	//! @return Return true whether at least one input value is applied
	bool apply(GraphInstance& graph,ApplyMode mode = Apply_Reset) const;

	//! @brief Fill this preset from a graph instance input values
	//! @param graph The source graph instance.
	//! @note In order to serialize input image filepaths, the
	//!		GlobalCallbacks::getImageFilepath callback must be defined.
	//!
	//! 'mInputValues' is filled w/ non default input values of 'graph'.
	//! 'mPackageUrl' is also set. All other members are not modified (label
	//!	and description).
	void fill(const GraphInstance& graph);
};


//! @brief Preset serialization (to UTF8 XML description) stream operator
std::ostream& operator<<(std::ostream& os,const Preset& preset);

//! @brief Parse preset from a XML preset element (\<sbspreset>)
//! @param[out] preset Preset description to fill from XML string
//! @param xmlString Pointer on UTF8 Null terminated string that contains
//!		serialized XML preset element \<sbspreset>
//! @return Return whether preset parsing succeed
bool parsePreset(Preset &preset,const char* xmlString);


//! @brief Container of presets
typedef vector<Preset> Presets;


//! @brief Presets serialization (to UTF8 XML description) stream operator
std::ostream& operator<<(std::ostream& os,const Presets& presets);

//! @brief Parse presets from a XML preset file (.SBSPRS)
//! @param[out] presets Container filled with loaded presets
//! @param xmlString Pointer on UTF8 Null terminated string that contains
//!		presets XML description
//! @return Return whether preset parsing succeed
bool parsePresets(Presets &presets,const char* xmlString);


}  // namespace SubstanceAir

#endif //_SUBSTANCE_AIR_FRAMEWORK_PRESET_H
