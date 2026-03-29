//! @file renderer.h
//! @brief The Substance renderer class, used to render instances
//! @author Christophe Soum - Allegorithmic
//! @copyright Allegorithmic. All rights reserved.

#ifndef _SUBSTANCE_AIR_FRAMEWORK_RENDERER_H
#define _SUBSTANCE_AIR_FRAMEWORK_RENDERER_H

#include "renderopt.h"
#include "package.h"
#include "callbacks.h"

#include <substance/version.h>




namespace SubstanceAir
{

namespace Details
{
	class RendererImpl;
}


//! @brief Class used to render graph instances
class Renderer
{
public:
	//! @brief Run options enumeration used as argument by run() method
	//! Options can be combined. Run_PreserveRun is only useful w/ Run_Replace
	//! and/or Run_First flags (useful w/ burst run() calls).
	enum RunOption
	{
		Run_Default      = 0,  //!< Synchronous, preserve previous push
		Run_Asynchronous = 1,  //!< Asynchronous computation
		Run_Replace      = 2,  //!< Discard computation on deprecated outputs
		Run_First        = 4,  //!< Run w/ maximum priority on other computation
		Run_PreserveRun  = 8   //!< In any case, preserve currently running job
	};


	//! @brief Default constructor
	//! @param renderOptions Optional render options. Allows to set initial
	//!		memory consumption budget and CPU usage.
	//! @param module Initial engine module
	//! @note Initial options can be override afterwards using setOptions().
	Renderer(const RenderOptions& renderOptions = RenderOptions(), void* module = nullptr);

	//! @brief Destructor
	//! @pre Grabbed render results created w/ this renderer must be deleted.
	//!
	//! If provided, calls RenderCallbacks::runRenderProcess() to release
	//!	Substance Engine. Allows deletion from the same thread used for render
	//!	processes (required by GPU Substance Engine versions).
	~Renderer();

	//! @brief Push graph instance current changes to render
	//! The current GraphInstance state is used to enqueue a render job: input
	//!	values and flags, output flags.
	//!
	//! Only the outputs of the graph flagged as dirty (outdated) are rendered
	//!	(use OutputInstance::flagAsDirty() to force rendering).<BR>
	//! When a GraphInstance is pushed for the first time, all outputs are
	//! considered dirty (and therefore are computed later).
	//! @note Note that the rendering process is not started by this command but
	//! with the run() method.
	void push(GraphInstance&);

	//! @brief Helper: Push graph instance arrays to render
	void push(const GraphInstances&);

	//! @brief Launch synchronous/asynchronous computation
	//! @param runOptions Combination of RunOption flags
	//! @param userData User data transmitted to output complete callback.
	//!		can be used to store pointer or other stuff.
	//! @post If synchronous mode selected (default): Render results are
	//!		available in output instances (OutputInstance::grabResult()).
	//! @return Return UID of render job or 0 if not pushed computation to run.
	//!
	//! Render previously pushed GraphInstance's. Does nothing if no
	//!	computation are pushed (no previous call to push() or no dirty
	//!	outputs in pushed instances).
	//!
	//! Both synchronous/asynchronous render are processed on an internal
	//! render thread by default (useful for GPU Substance Engine versions).
	//! This behavior can be overridden by RenderCallbacks::runRenderProcess().
	//!
	//! Returns after computation end or immediately if Run_Asynchronous mode
	//! is set and render is not processed on caller thread
	//!	(using RenderCallbacks::runRenderProcess() callback).
	UInt run(UInt runOptions = Run_Default,size_t userData = 0);

	//! @brief Cancel a computation
	//! @param runUid UID of the computation to cancel (returned by run())
	//! @return Return true if the job is retrieved (pending)
	bool cancel(UInt runUid);

	//! @brief Cancel all active/pending computations
	void cancelAll();

	//! @brief Return if a computation is pending
	//! @param runUid UID of the render job to retrieve state (returned by run())
	bool isPending(UInt runUid) const;

	//! @brief Hold rendering
	void hold();

	//! @brief Continue held rendering
	void resume();

	//! @brief Wait for all active/pending computations' completion
	//! @note Force resume of if rendering is held.
	void flush();

	//! @brief Restore graphics API states for render
	//! This member must be called only from runRenderProcess() callback
	//!	override.
	//! This MUST be called before renderFunction in the runRenderProcess callback
	//!	if the framework shares a user graphics API context (e.g. OpenGL context).
	//!     This is required to restore the internal state tracking of the engine.
	void restoreRenderStates();

	//! @brief Set new memory budget and CPU usage.
	//! @param renderOptions New render options to use.
	//!
	//! The new memory budget and CPU usage are applied on current/pending jobs.
	//! This function is thread safe.
	//!	@note If no computation is running, in order to apply the memory budget
	//!		a call to run() is necessary (possibly without pushed computations).
	//!		N.b. setOptions() is thread safe but run() is not.
	void setOptions(const RenderOptions& renderOptions);

	//! @brief Set per-renderer user callbacks
	//! @param callbacks Pointer on the user callbacks concrete structure
	//! 	instance that will be used for this renderer instance callbacks
	//! 	calls. Can be nullptr pointer: callbacks no more used.
	//! @warning A set per-renderer callback instance is used by all computation
	//! 	created by run() calls, even if another callback instance is set
	//!		just after. Then take care of RenderCallbacks instance lifetime
	//!		(to avoid issues, should be deleted AFTER Renderer instance).
	void setRenderCallbacks(RenderCallbacks* callbacks);

	//! @brief Switch engine implementation using dynamic-link library
	//! @param module Dynamic library to use, result of dlopen/LoadLibrary call.
	//!		Use nullptr to revert to default Engine implementation (Software).
	//!		The module must be valid since the Renderer is still using it.
	//! @note Flush all active/pending/held computations and
	//!		delete all pending (not grabbed) render results before switch.
	//! @pre Grabbed render results created w/ this renderer must be deleted.
	//! @return Return true if dynamic library loaded and compatible.
	//!
	//! Accept Engine w/ any platform (see platform.h).
	bool switchEngineLibrary(void* module);

	//! @brief Retrieve currently dynamic-linked lib version
	//! @return Returns SubstanceVersion object from active engine
	SubstanceVersion getCurrentVersion() const;

protected:
	Details::RendererImpl* mRendererImpl;
};


} // namespace SubstanceAir

#endif // _SUBSTANCE_AIR__RENDERER_H
