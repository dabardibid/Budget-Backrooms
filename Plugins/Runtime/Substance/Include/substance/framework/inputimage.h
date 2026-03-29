//! @file inputimage.h
//! @brief Substance Air image class used into image inputs
//! @author Christophe Soum - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_INPUTIMAGE_H
#define _SUBSTANCE_AIR_INPUTIMAGE_H

#include "platform.h"
#include "typedefs.h"

#include <substance/pixelformat.h>

struct SubstanceTexture_;
struct SubstanceTextureInput_;


namespace SubstanceAir
{

namespace Details
{
	struct InputImageToken;
}


//! @brief Substance Air image used into image inputs
//! InputImage are used by InputInstanceImage (input.h).
//! It contains image content used as input of graph instances.
//! Use InputImage::ScopedAccess to read/write texture buffer
class InputImage
{
public:
	//! @brief Shared pointer on InputImage instance type
	typedef shared_ptr<InputImage> SPtr;
	
	//! @brief User data, member only provided for user convenience
	size_t mUserData;

	//! @brief Build a new InputImage from texture description
	//! BLEND (default: textures in system memory) Substance Engine API 
	//! platform version ONLY.
	//! @param texture SubstanceTexture that describes the texture.
	//! @param bufferSize Size in bytes of the texture data buffer. Required
	//!		 and only used variable size formats (Substance_PF_JPEG).
	//!
	//!	A new buffer is automatically allocated to the proper 
	//!	size. If texture.buffer is nullptr the internal buffer is not
	//!	initialized (use ScopedAccess to modify its content). Otherwise,
	//!	texture.buffer is not nullptr, its content is copied into the 
	//!	internal buffer (texture.buffer is no more useful after this call).
	//!	If format is Substance_PF_JPEG, level0Width and level0Height members
	//!	are ignored (texture size is read from JPEG header) and bufferSize
	//!	is required.
	//! @see substance/texture.h
	//! @return Return an ImageInput instance or nullptr pointer if size or
	//!		format are not valid.
	static SPtr create(
		const SubstanceTexture_& texture,
		size_t bufferSize = 0);

	//! @brief Build a new InputImage from agnostic texture input description
	//! Substance Engine API multi-platform version: direct texture access.
	//! @param texture Platform specific texture description. The texture is
	//!		NOT copied and the ownership of the texture is NOT grabbed, 
	//!		must remains valid during whole InputImage instance lifetime
	//!		(including asynchronous rendering).
	//! @return Return an ImageInput instance.
	static SPtr create(const TextureInputAgnostic& textureInput);
	

	//! @brief Mutexed texture scoped access
	struct ScopedAccess
	{
		//! @brief Constructor from InputImage to access (thread safe)
		//! @param inputImage The input image to lock, cannot be nullptr pointer 
		//! @post The access to ImageInput is locked until destructor call.		
		ScopedAccess(const SPtr& inputImage);
		
		//! @brief Destructor, unlock InputImage access
		//! @warning Do not modify buffer outside ScopedAccess scope
		~ScopedAccess();
	
		#if !defined(AIR_PLATFORM_AGNOSTIC)

			//! @brief Accessor on texture description
			//! @warning Do not delete texture content (buffer or texture). 
			//!		However its content can be freely modified inside ScopedAccess 
			//!		scope.
			const SubstanceTexture* operator->() const 
				{ return &castToConcrete(getTextureInput()).mTexture; }

		#endif // if !defined(AIR_PLATFORM_AGNOSTIC)

		//! @brief Accessor on texture input platform agnostic description
		//! @warning Do not delete texture content (buffer or texture). 
		//!		However its content can be freely modified inside ScopedAccess 
		//!		scope.
		const TextureInputAgnostic& getTextureInput() const;
		
		//! @brief Helper: returns buffer content size in bytes
		//! @pre Only valid w/ BLEND Substance Engine API platform
		//!		(system memory texture).
		size_t getSize() const;
		
	protected:
		const SPtr mInputImage;
	};
	
	//! @brief Destructor
	//! @warning Do not delete this class directly if it was already set
	//!		into a InputImageImage: it can be still used in rendering process.
	//!		Use shared pointer mechanism instead.
	~InputImage();

	bool resolveDirty();                            //!< Internal use only
	Details::InputImageToken* getToken() const;     //!< Internal use only
protected:
	bool mDirty;
	Details::InputImageToken *mInputImageToken;
	InputImage(Details::InputImageToken*);
private:
	InputImage(const InputImage&);
	const InputImage& operator=(const InputImage&);
};


} // namespace SubstanceAir

#endif //_SUBSTANCE_AIR_INPUTIMAGE_H
