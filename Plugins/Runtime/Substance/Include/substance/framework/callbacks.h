//! @file callbacks.h
//! @brief Substance Air callbacks Global & per-Renderer abstract structures
//! @author Christophe Soum - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.
//!
//! Abstract base structures of global & per-renderer user callbacks

#ifndef _SUBSTANCE_AIR_CALLBACKS_H
#define _SUBSTANCE_AIR_CALLBACKS_H

#include "typedefs.h"

#include <substance/engineid.h>


struct SubstanceDevice_;


namespace SubstanceAir
{

class GraphInstance;
class OutputInstance;
class InputInstanceImage;
class InputImage;


//! @brief Pointer to function used as render function definition
typedef void (*RenderFunction)(void *);


//! @brief Profile events
enum class ProfileEvent 
{
	LinkBegin,
	LinkEnd,
	ComputeBegin,
	ComputeEnd
};


//! @brief Abstract base structure of per-Renderer user callbacks
//! Inherit from this base struct and overload the virtual methods to be
//! notified by framework per-renderer callbacks.
//!
//! Use concrete callbacks instance in Renderer::setRenderCallbacks() method.
//! @warning Callback methods can be called from a different thread, not
//!		necessary the user thread (that call Renderer::run()).
//!
//! Available callbacks:
//!  - Output computed (render result available)
struct RenderCallbacks
{
	//! @brief Destructor
	virtual ~RenderCallbacks();

	//! @brief Output computed callback (render result available)
	//! @param runUid The UID of the corresponding computation (returned by
	//! 	Renderer::run()).
	//! @param userData The user data set at corresponding run
	//!		(as second argument Renderer::run(xxx,userData))
	//! @param graphInstance Pointer on output parent graph instance
	//! @param outputInstance Pointer on computed output instance w/ new render
	//!		result just available (use OutputInstance::grabResult() to grab it).
	//!
	//! Called each time an new render result is just available.
	//! By default call the function below.
	virtual void outputComputed(
		UInt runUid,
		size_t userData,
		const GraphInstance* graphInstance,
		OutputInstance* outputInstance);

	//! @brief Job computed callback
	//! @param runUid The UID of the corresponding computation (returned by
	//! 	Renderer::run()).
	//! @param userData The user data set at corresponding run
	//!		(as second argument Renderer::run(xxx,userData))
	//! This is called every time a job is completed by the renderer.
	virtual void jobComputed(
		UInt runUid,
		size_t userData);

	//! @brief Render process execution callback
	//! @param renderFunction Pointer to function to call.
	//! @param renderParams Parameters of the function.
	//! @return Return true if render process execution is handled. If
	//!		returns false once, the renderer uses internal thread until
	//!		eventual engine switch.
	//!
	//! Allows custom thread allocation for rendering process. If handled,
	//! must call renderFunction w/ renderParams.
	//!	This function returns when all pending render jobs are processed.
	//! Called each time a batch a jobs need to be rendered AND at renderer
	//! deletion (guarantee thread homogeneity for GPU Substance Engine
	//!	versions).
	//!
	//! IMPORTANT: If the used graphics API context (e.g. OpenGL context)
	//! is used elsewhere, the renderer's restoreRenderStates() member
	//! MUST be called before renderFunction to avoid state tracking issues.
	//!
	//! Default implementation returns false: use internal thread.
	virtual bool runRenderProcess(
		RenderFunction renderFunction,
		void* renderParams);

	//! @brief Fill Substance Engine Device callback
	//! @param platformId The id corresponding to current engine API platform
	//! @param substanceDevice Pointer on substance device to fill
	//!		Concrete device type depends on engine API platform.
	//! @note Can be discarded on Engine w/ BLEND platform (textures in system
	//!		memory: default). Mandatory on some platforms.
	virtual void fillSubstanceDevice(
		SubstanceEngineIDEnum platformId,
		SubstanceDevice_ *substanceDevice);

	//! @brief Profiling events launched from computation thread
	virtual void profileEvent(ProfileEvent) {}
};


//! @brief Abstract base structure of global user callbacks
//! Inherit from this base struct and overload the virtual methods to be
//! notified by framework base callbacks.
//!
//! Use concrete callbacks instance in GlobalCallbacks::setInstance() static
//! method.
//! @warning Callback methods can be called from a different thread, not
//!		necessary the user thread (that call Renderer::run()). getEnabledMask()
//!		must be also overridden in order to enable callbacks actually overridden.
//! @note These callbacks should be overridden once, they are used by all
//!		Renderer instance.
//!
//! Available callbacks:
//!  - Evict cache system (memory/mass storage swapping)
//!  - User memory allocation
//!  - Preset input image loading/serialization
struct GlobalCallbacks
{

	//! @brief Callbacks groups, enumation used by getEnabledMask()
	enum Enabled
	{
		Enable_RenderCache = 0x1,   //!< Evict/Fetch/Remove cache callbacks
		Enable_UserAlloc   = 0x2    //!< DEPRECATED: has now default implementation
	};


	//! @brief Set global (static) user callbacks
	//! @param callbacks Pointer on the user callbacks concrete structure
	//! 	instance that will be used for the global callbacks call.
	//! @warning Global callback instance must be set before any render and
	//!		never modified. GlobalCallbacks instance should be deleted AFTER
	//! 	Renderers instances.
	static void setInstance(GlobalCallbacks* callbacks);

	//! @brief Accessor on global callbacks (can be nullptr: not defined)
	static GlobalCallbacks* getInstance() { return mInstance; }


	//! @brief Destructor
	virtual ~GlobalCallbacks();

	//! @brief Enabled user callbacks mask
	//! @return Returns a combination of Enabled enums, indicates which
	//! 	callbacks are really overridden by the user.
	//!
	//! In order to override callbacks, the user must override this method in
	//! concrete GlobalCallbacks structures and make it return the enabled
	//! enums.
	virtual unsigned int getEnabledMask() const;


	//! @brief Evict cache callback (interm. result render need to be evicted)
	//! @param cacheItemUid The UID of the cache item to evict
	//! @param cacheBuffer The buffer of the corresponding cache item. Must
	//!		be stored by user.
	//! @param cacheBufferSize The buffer size of the corresponding cache item
	//! @note This callback must be implemented in concrete GlobalCallbacks
	//! 	structures if getEnabledMask() returns Enable_RenderCache bit.
	//!
	//! Called each time engine need to evict an intermediary result
	virtual void renderCacheEvict(
		UInt64 cacheItemUid,
		const void *cacheBuffer,
		size_t cacheBufferSize);

	//! @brief Fetch cache callback (interm. result render need to be fetched)
	//! @param cacheItemUid The UID of the cache item to retrieve
	//! @param cacheBuffer The buffer of the corresponding cache item.
	//! 	Must be filled w/ the content provided by the previous
	//! 	renderCacheEvict() call w/ same 'cacheItemUid'.
	//! @param cacheBufferSize The buffer size of the corresponding cache item
	//! @note This callback must be implemented in concrete GlobalCallbacks
	//! 	structures if getEnabledMask() returns Enable_RenderCache bit.
	//!
	//! Called each time engine need to fetch an intermediary result
	virtual void renderCacheFetch(
		UInt64 cacheItemUid,
		void *cacheBuffer,
		size_t cacheBufferSize);

	//! @brief Remove cache callback (interm. result render no more necessary)
	//! @param cacheItemUid The UID of the cache item to remove from cache.
	//! @note This callback must be implemented in concrete GlobalCallbacks
	//! 	structures if getEnabledMask() returns Enable_RenderCache bit.
	//!
	//! Called each time an intermediary result is deprecated
	virtual void renderCacheRemove(
		UInt64 cacheItemUid);


	//! @brief User system memory allocation callback
	//! @param bytesCount Number of bytes to allocate.
	//! @param alignment Required buffer address alignment (in bytes).
	//! @return Return the corresponding buffer.
	//!
	//! This callback allows the use of user-defined dynamic memory allocation.
	virtual void* memoryAlloc(
		size_t bytesCount,
		size_t alignment);

	//! @brief User system memory de-allocation callback
	//! @param bufferPtr Buffer to free.
	//!
	//! This callback allows to free buffers allocated with the malloc()
	//! callback.
	virtual void memoryFree(
		void* bufferPtr);


	//! @brief Preset apply, image loading callback
	//! @param[out] inst The image input instance to set loaded image (using
	//! 	InputInstanceImage::setImage).
	//! @param filepath Filepath of the image to load in UTF-8 format,
	//!		<b>relative to preset filepath</b>.
	//! @warning The filepath is relative to loaded preset file location.
	//!
	//! This callback is called for each input image set by an applied preset
	//! (Preset::apply). The image loading operation must be done by this
	//! callback.
	virtual void loadInputImage(
		InputInstanceImage& inst,
		const string& filepath);

	//! @brief Preset filling, image filepath callback
	//! @param inputImage The image input to retrieve corresponding bitmap
	//! 	filepath. The InputImage::mUserData can be useful to tag input
	//!		images and retrieve filepath in a easier way.
	//! @return Filepath of the bitmap corresponding to inputImage in UTF-8
	//!		format. The filepath must be <b>relative to preset filepath</b>.
	//!		Return an empty string to skip this input in the preset.
	//! @warning The returned filepath must be relative to saved preset file
	//!		location.
	//!
	//! This callback is called for each input image to serialize during a
	//!	preset creation (Preset::fill).
	virtual string getImageFilepath(
		const InputImage& inputImage);

private:
	//! @brief Current User global callbacks instance (can be nullptr, none)
	static GlobalCallbacks* mInstance;
};


} // namespace SubstanceAir

#endif //_SUBSTANCE_AIR_CALLBACKS_H
