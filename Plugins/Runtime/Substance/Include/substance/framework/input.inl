//! @file input.inl
//! @brief Substance Air input inline impl.
//! @author Antoine Gonzalez - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.


#include <algorithm>


//! @brief Internal use only
template <typename T> 
SubstanceAir::InputInstanceBase* 
SubstanceAir::InputDescNumerical<T>::instantiate(GraphInstance& parent) const
{
	return AIR_NEW(InputInstanceNumerical<T>)(*this,parent);
}


template <typename T> 
SubstanceAir::InputInstanceNumerical<T>::InputInstanceNumerical(
		const Desc& desc,
		GraphInstance& parent) :
	InputInstanceNumericalBase(desc,parent),
	mValue(desc.mDefaultValue)
{
}


template <typename T> 
const typename SubstanceAir::InputInstanceNumerical<T>::Desc& 
SubstanceAir::InputInstanceNumerical<T>::getDesc() const
{
	return static_cast<const Desc&>(mDesc);
}


template <typename T> 
void SubstanceAir::InputInstanceNumerical<T>::setValue(const T& src)
{
	if (mValue!=src)
	{
		mValue = src;
		postModification();
	}
}


template <typename T> 
SubstanceAir::string SubstanceAir::InputInstanceNumerical<T>::getValueAsString() const
{
	stringstream sstr;
	sstr << getValue();
	return sstr.str();
}

	
template <typename T> 
bool SubstanceAir::InputInstanceNumerical<T>::setValue(const string& src)
{
	T value;
	stringstream sstr(src);
	sstr >> value;
	
	if (!sstr.fail())
	{
		setValue(value);
		return true;
	}

	return false;
}


//! @brief Helper: reset value to default
template <typename T>
void SubstanceAir::InputInstanceNumerical<T>::reset()
{
	setValue(getDesc().mDefaultValue);
}


//! @brief Helper: is set to non default value
template <typename T>
bool SubstanceAir::InputInstanceNumerical<T>::isNonDefault() const
{
	return getDesc().mDefaultValue!=mValue;
}

