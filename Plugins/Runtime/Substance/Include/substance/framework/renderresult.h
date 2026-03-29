//! @file renderresult.h
//! @brief Substance Air rendering result
//! @author Antoine Gonzalez - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_RENDERRESULT_H
#define _SUBSTANCE_AIR_RENDERRESULT_H

#include "platform.h"
#include "typedefs.h"

#include <substance/iotype.h>

#if !defined(AIR_PLATFORM_AGNOSTIC)
	#include <substance/context.h>
#endif //if !defined(AIR_PLATFORM_AGNOSTIC)


struct SubstanceContext_;


namespace SubstanceAir
{
namespace Details
{
	class Engine;
}

struct RenderCallbacks;


//! @brief Substance base result struct
struct RenderResultBase
{
	//! @brief Type of the result
	//! @see SubstanceIOType enum in substance/iotype.h
	const SubstanceIOType mType;

	virtual ~RenderResultBase() {}

	//! @brief Return if the type of this result is an image type
	//! Helper, can be deduced from mType
	bool isImage() const { return mType==Substance_IOType_Image; }

	//! @brief Return if the type of this result is numerical
	//! Helper, can be deduced from mType
	bool isNumerical() const { return !isImage(); }

protected:
	RenderResultBase(SubstanceIOType type) : mType(type) {}

private:
	RenderResultBase(const RenderResultBase&);
	const RenderResultBase& operator=(const RenderResultBase&);
};


//! @brief Substance image result struct (texture result)
//! @invariant Result must be deleted or content must be grabbed before
//!		corresponding renderer deletion or renderer engine implementation is
//!		switch.
struct RenderResultImage : RenderResultBase
{
	//! @brief Constructor, internal use only
	RenderResultImage(const TextureAgnostic&,SubstanceContext_*,Details::Engine*);

	//! @brief Destructor
	//! Delete texture content contained in mSubstanceTexture
	//! if not previously grabbed (releaseBuffer() or releaseTexture()).
	~RenderResultImage();

	//! @brief Accessor on the Substance context to use for releasing the
	//!		texture content.
	SubstanceContext_* getContext() const { return mContext; }

	#if !defined(AIR_PLATFORM_AGNOSTIC)

		//! @brief Return if hold a texture that match current platform
		bool isPlatformMatch(AIR_PLATFORM_DUMMYPARAM) const
			{ (void)_dummyParam; return SubstanceAir::isPlatformMatch(mTextureAgnostic); }

		//! @brief Accessor on the result texture
		//! Contains texture content (buffer on BLEND platform)
		//! @pre Texture created w/ matching engine platform
		//!		(isPlatformMatch()==true)
		//! @warning Read-only accessor, do not delete the texture content.
		//! @see substance/texture.h
		const SubstanceTexture& getTexture(AIR_PLATFORM_DUMMYPARAM) const
			{ (void)_dummyParam; return castToConcrete(mTextureAgnostic); }

		#ifdef SUBSTANCE_PLATFORM_BLEND
			//! @brief Grab the pixel data buffer
			//! @pre Texture created w/ BLEND platform engine
			//!		(isPlatformMatch()==true)
			//! @warning The ownership of the buffer is given to the caller.
			//! 	The buffer must be freed by alignedFree() (see memory.h).
			//! @return Return the buffer, or nullptr if already released
			void* releaseBuffer()
				{ return castToConcrete(releaseTexture()).buffer; }
		#endif //ifdef SUBSTANCE_PLATFORM_BLEND

	#endif // if !defined(AIR_PLATFORM_AGNOSTIC)

	//! @brief Grab texture content ownership, platform agnostic
	//! @pre The texture content was not previously grabbed (haveOwnership()==true)
	//! @return Return the texture. The ownership of the texture content
	//!		is transferred to the caller.
	TextureAgnostic releaseTexture();

	//! @brief Accessor on the result texture, platform agnostic
	//! @warning Read-only accessor, do not delete the texture content.
	//! @return Return the texture or invalid texture if renderer is deleted
	//!		or its engine is switched.
	const TextureAgnostic& getTextureAgnostic() const
		{ return mTextureAgnostic; }

	//! @brief Return if the render result still have ownership on content
	bool haveOwnership() const { return mHaveOwnership; }

	//! @brief Internal use
	Details::Engine* getEngine() const { return mEngine; }

protected:
	TextureAgnostic mTextureAgnostic;
	bool mHaveOwnership;
	SubstanceContext_* mContext;
	Details::Engine* mEngine;
};


//! @brief Base class of all numerical result
struct RenderResultNumericalBase : RenderResultBase
{
	//! @brief Get value as string
	virtual string getValueAsString() const = 0;

	//! @brief Accessor as raw value
	BinaryData getRawValue() const
	{
		const unsigned char* const dataptr = (const unsigned char*)getRawData();
		return BinaryData(dataptr,dataptr+getRawSize());
	}

	virtual const void* getRawData() const = 0;   //!< Internal use only
	virtual size_t getRawSize() const = 0;	      //!< Internal use only

protected:
	RenderResultNumericalBase(SubstanceIOType type) : RenderResultBase(type) {}
};


//! @brief Concrete class of numerical result
//! T is the concrete input type, and be one of the following:
//!  - float     : mType == Substance_IOType_Float
//!  - Vec2Float : mType == Substance_IOType_Float2
//!  - Vec3Float : mType == Substance_IOType_Float3
//!  - Vec4Float : mType == Substance_IOType_Float4
//!  - int       : mType == Substance_IOType_Integer
//!  - Vec2Int   : mType == Substance_IOType_Integer2
//!  - Vec3Int   : mType == Substance_IOType_Integer3
//!  - Vec4Int   : mType == Substance_IOType_Integer4
template <typename T>
struct RenderResultNumerical : RenderResultNumericalBase
{
	typedef T Type;

	//! @brief Result value
	const T mValue;

	//! @brief Constructor, internal use only
	RenderResultNumerical(SubstanceIOType type,const void *value) :
		RenderResultNumericalBase(type),
		mValue(*(const T*)value) {}

	//! @brief Get value as string
	string getValueAsString() const
	{
		stringstream sstr;
		sstr << mValue;
		return sstr.str();
	}

	const void* getRawData() const { return &mValue; }    //!< Internal use only
	size_t getRawSize() const { return sizeof(mValue); }  //!< Internal use only
};


typedef RenderResultNumerical<float>     RenderResultFloat;  //!< Substance_IOType_Float
typedef RenderResultNumerical<Vec2Float> RenderResultFloat2; //!< Substance_IOType_Float2
typedef RenderResultNumerical<Vec3Float> RenderResultFloat3; //!< Substance_IOType_Float3
typedef RenderResultNumerical<Vec4Float> RenderResultFloat4; //!< Substance_IOType_Float4
typedef RenderResultNumerical<int>       RenderResultInt;    //!< Substance_IOType_Integer
typedef RenderResultNumerical<Vec2Int>   RenderResultInt2;   //!< Substance_IOType_Integer2
typedef RenderResultNumerical<Vec3Int>   RenderResultInt3;   //!< Substance_IOType_Integer3
typedef RenderResultNumerical<Vec4Int>   RenderResultInt4;   //!< Substance_IOType_Integer4


} // namespace SubstanceAir

#endif //_SUBSTANCE_AIR_RENDERRESULT_H
