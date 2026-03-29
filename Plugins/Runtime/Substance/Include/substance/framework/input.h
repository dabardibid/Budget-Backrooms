//! @file input.h
//! @brief Substance Air input definitions
//! @author Antoine Gonzalez - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_INPUT_H
#define _SUBSTANCE_AIR_FRAMEWORK_INPUT_H

#include "inputimage.h"
#include "channel.h"

#include <substance/inputdesc.h>



#include <utility>

namespace SubstanceAir
{

struct GraphDesc;
class GraphInstance;
class InputInstanceBase;

namespace Details
{
	struct InstantiateInput;
}


//! @brief The list of input widgets available in Designer
enum InputWidget
{
	Input_NoWidget,         //!< Widget not defined
	Input_Slider,           //!< Simple widget slider
	Input_Angle,            //!< Angle widget
	Input_Color,            //!< Color widget
	Input_Togglebutton,     //!< On/Off toggle button or checkbox
	Input_Combobox,         //!< Drop down list
	Input_Image,            //!< Image input widget
	Input_Position,         //!< Position input widget
	Input_INTERNAL_COUNT
};


//! @brief Base class of all Substance input descriptions
//! Input descriptions are members of GraphDesc.
struct InputDescBase
{
	//! @brief User-readable identifier of the input
	string mIdentifier;

	//! @brief 32 bits unsigned Unique Identifier
	UInt mUid;

	//! @brief Type of the input
	//! @see SubstanceIOType enum in substance/iotype.h
	SubstanceIOType mType;

	//! @brief Label of this input (UI) UTF8 format
	string mLabel;

	//! @brief List of output UIDs altered by this input
	//! UIDs are sorted (allows binary search).
	//!
	//! When this input is modified, the outputs listed here are flagged as
	//! dirty (outdated) and are recomputed at next rendering.
	//! @warning Dynamic output components composition
	//! 	(OutputFormat::Component::outputUid) is not taken into account.
	Uids mAlteredOutputUids;

	//! @brief This input is heavy duty by default
	bool mIsDefaultHeavyDuty;

	//! @brief Label of the group of this input (UI) UTF8 format
	//! Empty string if this input is not grouped.
	string mGuiGroup;

	//! @brief Detailed description of this input (UI) UTF8 format
	string mGuiDescription;

	//! @brief Display mode: if true prefers display this input as a graph pin
	//! Meaningful only for integration with a graph based layout.
	bool mShowAsPin;

	//! @brief GUI Widget type best suited to visualize this input
	InputWidget mGuiWidget;

	//! @brief Expression that allows to control input visibility status (UI).
	//! Empty if no conditional visibility.
	//! @see See visibleif.h header to evaluate this expression. Depend on
	//!		current (other) input values of the parent graph.
	string mGuiVisibleIf;

	//! @brief User tag of this input (UI/integration purpose only) UTF8 format
	string mUserTag;

	//! @brief Channel usages, as strings
	//! Useful for elements of mChannels that are Channel_UNKNOWN: user specific syntax.
	//! Meaningful for inputs interpreted as material channel (image or numeric).
	vector<string> mChannelsStr;

	//! @brief Usages of the input as ChannelUse enumerators, or Channel_UNKNOWN if user specific
	//! Meaningful for inputs interpreted as material channel (image or numeric).
	vector<ChannelUse> mChannels;

	//! @brief Virtual destructor
	virtual ~InputDescBase();

	//! @brief Return if the type of this input is numerical
	//! Helper, can be deduced from mType
	virtual bool isNumerical() const { return false; }

	//! @brief Return if the type of this input is an image type
	//! Helper, can be deduced from mType
	virtual bool isImage() const { return false; }

	//! @brief Return if the type of this input is a string type
	//! Helper, can be deduced from mType
	virtual bool isString() const { return false; }

	//! @brief Internal use only
	virtual const void* getRawDefault() const { return nullptr; }
protected:
	virtual InputDescBase* clone() const = 0;
	virtual InputInstanceBase* instantiate(GraphInstance&) const = 0;

	friend struct GraphDesc;
	friend class GraphInstance;
	friend class PackageStackDesc;
	friend struct Details::InstantiateInput;
};


//! @brief Base class of numerical Substance input description
struct InputDescNumericalBase : public InputDescBase
{
	//! @brief Vector channels labels (UI) UTF8 format
	//! @note Only relevant if widget is Input_Slider. If empty, default labels
	//!		should be used (X,Y,Z,W).
	vector<string> mGuiVecLabels;

	//! @brief Pantone Spot Color info
	//! @note  Only valid for color-type float3 and float4
	//! @note  This is a string formatted like "3011,   50Z" for instance
	//! @note  First field is the Spot Color Book ID, second field is the Spot Color ID
	string mSpotColorInfo;

	//! @brief Return if the type of this input is numerical
	//! Helper, can be deduced from mType
	bool isNumerical() const { return true; }
};


//! @brief Concrete class of numerical Substance input description
//! T is the concrete input type, and be one of the following:
//!  - float     : mType == Substance_IOType_Float
//!  - Vec2Float : mType == Substance_IOType_Float2
//!  - Vec3Float : mType == Substance_IOType_Float3
//!  - Vec4Float : mType == Substance_IOType_Float4
//!  - int       : mType == Substance_IOType_Integer
//!  - Vec2Int   : mType == Substance_IOType_Integer2
//!  - Vec3Int   : mType == Substance_IOType_Integer3
//!  - Vec4Int   : mType == Substance_IOType_Integer4
//!
//! All members are for UI purposes only.
template <typename T>
struct InputDescNumerical : public InputDescNumericalBase
{
	typedef T Type;
	typedef vector<std::pair<T,string> > EnumValues;

	//! @brief Default input value
	T mDefaultValue;

	//! @brief Minimum value (UI hint only)
	//! @note Only relevant if widget is Input_Slider
	T mMinValue;

	//! @brief Maximum value (UI hint only)
	//! @note Only relevant if widget is Input_Slider
	T mMaxValue;

	//! @brief Slider step size (UI hint only)
	//! @note Only relevant if widget is Input_Slider
	float mSliderStep;

	//! @brief Should the slider clamp the value? (UI hint only)
	//! @note Only relevant if widget is Input_Slider
	bool mSliderClamp;

	//! @brief If non-empty, the labels to use for False (unchecked) and True (checked) values
	//! @note Only relevant if widget is Input_Togglebutton
	string mLabelFalse, mLabelTrue;

	//! @brief Enumeration: pairs of value and label in UTF8 format (UI only)
	//! @note Only relevant if widget is Input_Combobox
	EnumValues mEnumValues;

	//! @brief Internal use only
	const void* getRawDefault() const { return &mDefaultValue; }
protected:
	InputDescBase* clone() const { return AIR_NEW(InputDescNumerical<T>)(*this); }
	InputInstanceBase* instantiate(GraphInstance&) const;
};


typedef InputDescNumerical<float>     InputDescFloat;  //!< Substance_IOType_Float
typedef InputDescNumerical<Vec2Float> InputDescFloat2; //!< Substance_IOType_Float2
typedef InputDescNumerical<Vec3Float> InputDescFloat3; //!< Substance_IOType_Float3
typedef InputDescNumerical<Vec4Float> InputDescFloat4; //!< Substance_IOType_Float4
typedef InputDescNumerical<int>       InputDescInt;    //!< Substance_IOType_Integer
typedef InputDescNumerical<Vec2Int>   InputDescInt2;   //!< Substance_IOType_Integer2
typedef InputDescNumerical<Vec3Int>   InputDescInt3;   //!< Substance_IOType_Integer3
typedef InputDescNumerical<Vec4Int>   InputDescInt4;   //!< Substance_IOType_Integer4


//! @brief Concrete class of Image Substance input description
//! mType == Substance_IType_Image
struct InputDescImage : public InputDescBase
{
	//! @brief Color image flag
	//! True if the input image is color, false otherwise (only a hint,
	//!	conversions are performed automatically).
	bool mIsColor : 1;

	//! @brief Floating point format
	//! True if the input prefers image w/ floating point format (FP16 or FP32)
	bool mIsFPFormat : 1;

	//! @brief Default color used for this input image (R/G/B/A or L/L/L/1)
	//! If not filled in the SBSAR, { 0, 0, 0, 0 } is used.
	Vec4Float mDefaultColor;

	//! @brief Return if the type of this input is image
	//! Helper, can be deduced from mType
	bool isImage() const { return true; }

	//! @brief Accessor for the default channel usage for this input image
	//! Returns the default channel type
	ChannelUse defaultChannelUse() const;

	//! @brief Accessor for the default channel usage for this input image in string format
	//! Returns the default channel type as a string
	string defaultChannelUseStr() const;

	//! @brief Checks if the channel use was specified by the user
	//! Returns if the user set at least one channel type this input image is intended
	//!	to be used for.
	bool channelUseSpecified() const { return mChannels.size() != 0; }

	//! @brief Default constructor
	InputDescImage()
		: mIsColor(true)
		, mIsFPFormat(false)
		, mDefaultColor(0.f, 0.f, 0.f, 0.f)
	{
	}

protected:
	InputDescBase* clone() const { return AIR_NEW(InputDescImage)(*this); }
	InputInstanceBase* instantiate(GraphInstance&) const;
};


//! @brief Concrete class of String Substance input description
//! mType == Substance_IType_String
struct InputDescString : public InputDescBase
{
	//! @brief Default value in UTF-8 format
	string mDefaultValue;


	//! @brief Return if the type of this input is string
	//! Helper, can be deduced from mType
	bool isString() const { return true; }

	//! @brief Default constructor
	InputDescString()
	{
	}

	//! @brief Parameterized constructor
	InputDescString(const char* defaultValue)
		: mDefaultValue(defaultValue)
	{
	}

protected:
	InputDescBase* clone() const { return AIR_NEW(InputDescString)(*this); }
	InputInstanceBase* instantiate(GraphInstance&) const;
};


//! @brief Base class of all Substance input instances
//! Input instances are members of GraphInstance. Each InputInstanceBase
//! corresponds to an InputDesc, it contains the current tweak value. To modify
//! or read this value, downcast and use the concrete class that corresponds to
//! the input type (mDesc.mType).
class InputInstanceBase
{
public:
	//! @brief Reference on corresponding input description
	const InputDescBase &mDesc;

	//! @brief Reference on parent graph instance
	GraphInstance &mParentGraph;

	//! @brief Flag this input to be cached
	//! Outputs recomputed after cached input change are faster to regenerate.
	//!
	//! Default: false
	bool mUseCache;

	//! @brief Flag this input as heavy duty
	//! Allows to separate the inputs that cause a big change in the texture
	//! (randomseed...). Heavy duty inputs are never cached: the impact on the
	//! cache is too important.
	//!
	//! Default: initialized w/ mDesc.mIsDefaultHeavyDuty.
	bool mIsHeavyDuty;

	//! @brief The result of the visible if evaluation. Defaults to true
	bool IsVisible() const;

	//! @brief Helper: reset value to default
	virtual void reset() = 0;

	//! @brief Helper: is set to non default value
	virtual bool isNonDefault() const = 0;

	virtual ~InputInstanceBase();

	//! @brief Internal use only
	virtual bool isModified(const void*) const = 0;
protected:
	InputInstanceBase(const InputDescBase&,GraphInstance&);
	void postModification();
private:
	InputInstanceBase(const InputInstanceBase&);
	const InputInstanceBase& operator=(const InputInstanceBase&);
};


//! @brief Base class of all numerical Substance input instances
class InputInstanceNumericalBase : public InputInstanceBase
{
public:
	//! @brief Corresponding description type
	typedef InputDescNumericalBase Desc;

	//! @brief Accessor on concrete description class
	const Desc& getDesc() const;

	//! @brief Get value as string
	virtual string getValueAsString() const = 0;

	//! @brief Accessor as raw value
	BinaryData getRawValue() const;

	//! @brief Modify input value
	//! From string (use lexical cast)
	//! @return Return true if cast succeed
	virtual bool setValue(const string&) = 0;

	bool isModified(const void*) const;           //!< Internal use only
	virtual const void* getRawData() const = 0;   //!< Internal use only
	virtual size_t getRawSize() const = 0;	      //!< Internal use only
protected:
	InputInstanceNumericalBase(const InputDescBase&,GraphInstance&);
};


//! @brief Concrete class of numerical Substance input instances
//! T is the concrete input type. It can be one of the following:
//!  - float     : mDesc.mType == Substance_IOType_Float
//!  - Vec2Float : mDesc.mType == Substance_IOType_Float2
//!  - Vec3Float : mDesc.mType == Substance_IOType_Float3
//!  - Vec4Float : mDesc.mType == Substance_IOType_Float4
//!  - int       : mDesc.mType == Substance_IOType_Integer
//!  - Vec2Int   : mDesc.mType == Substance_IOType_Integer2
//!  - Vec3Int   : mDesc.mType == Substance_IOType_Integer3
//!  - Vec4Int   : mDesc.mType == Substance_IOType_Integer4
template <typename T>
class InputInstanceNumerical : public InputInstanceNumericalBase
{
public:
	//! @brief Corresponding description type
	typedef InputDescNumerical<T> Desc;

	//! @brief Accessor on concrete description class
	const Desc& getDesc() const;

	//! @brief Accessor on current value
	const T& getValue() const { return mValue; }

	//! @brief Modify input value
	//! From concrete type
	void setValue(const T&);

	//! @brief Get value as string
	string getValueAsString() const;

	//! @brief Modify input value
	//! From string (use lexical cast)
	//! @return Return true if cast succeed
	bool setValue(const string&);

	//! @brief Helper: reset value to default
	void reset();

	//! @brief Helper: is set to non default value
	bool isNonDefault() const;

	const void* getRawData() const { return &mValue; }    //!< Internal use only
	size_t getRawSize() const { return sizeof(mValue); }  //!< Internal use only
protected:
	T mValue;
	InputInstanceNumerical(const Desc&,GraphInstance&);

	friend struct InputDescNumerical<T>;
};


typedef InputInstanceNumerical<float>     InputInstanceFloat;  //!< Substance_IOType_Float
typedef InputInstanceNumerical<Vec2Float> InputInstanceFloat2; //!< Substance_IOType_Float2
typedef InputInstanceNumerical<Vec3Float> InputInstanceFloat3; //!< Substance_IOType_Float3
typedef InputInstanceNumerical<Vec4Float> InputInstanceFloat4; //!< Substance_IOType_Float4
typedef InputInstanceNumerical<int>       InputInstanceInt;    //!< Substance_IOType_Integer
typedef InputInstanceNumerical<Vec2Int>   InputInstanceInt2;   //!< Substance_IOType_Integer2
typedef InputInstanceNumerical<Vec3Int>   InputInstanceInt3;   //!< Substance_IOType_Integer3
typedef InputInstanceNumerical<Vec4Int>   InputInstanceInt4;   //!< Substance_IOType_Integer4


//! @brief Concrete class of Image Substance input instances
//! mDesc.mType == Substance_IType_Image
class InputInstanceImage : public InputInstanceBase
{
public:
	//! @brief Corresponding description type
	typedef InputDescImage Desc;

	//! @brief Accessor on concrete description class
	const Desc& getDesc() const;

	//! @brief Set a new image content (keep ownership)
	void setImage(const InputImage::SPtr&);

	//! @brief Accessor on current image content (can return nullptr shared ptr)
	const InputImage::SPtr& getImage() const { return mInputImage; }

	//! @brief Helper: reset value to default
	void reset();

	//! @brief Helper: is set to non default value
	bool isNonDefault() const;

	//! @brief Internal use only
	bool isModified(const void*) const;
protected:
	InputImage::SPtr mInputImage;
	mutable bool mPtrModified;
	InputInstanceImage(const Desc&,GraphInstance&);

	friend struct InputDescImage;
};


//! @brief Concrete class of String Substance input instances
//! mDesc.mType == Substance_IType_String
class InputInstanceString : public InputInstanceBase
{
public:
	//! @brief Corresponding description type
	typedef InputDescString Desc;

	//! @brief Accessor on concrete description class
	const Desc& getDesc() const;

	//! @brief Set a new string value (UTF-8 format)
	void setString(const string&);

	//! @brief Accessor on current string content (UTF-8 format)
	const string& getString() const { return mValue; }

	//! @brief Helper: reset value to default
	void reset();

	//! @brief Helper: is set to non default value
	bool isNonDefault() const;

	//! @brief Internal use only
	bool isModified(const void*) const;
protected:
	string mValue;
	mutable bool mIsModified;
	InputInstanceString(const Desc&,GraphInstance&);

	friend struct InputDescString;
};


//! @brief Helper: return the number of components of numeric types
//! @return Number of components or 0 if not a numeric type
size_t getComponentsCount(SubstanceIOType type);

//! @brief Helper: Return if is a float type
bool isFloatType(SubstanceIOType type);


} // namespace SubstanceAir


#include "input.inl"


#endif //_SUBSTANCE_AIR_FRAMEWORK_INPUT_H
