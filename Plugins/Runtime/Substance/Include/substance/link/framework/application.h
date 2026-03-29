//! @file application.h
//! @brief Base application type for Link
//! @author Galen Helfter - Allegorithmic
//! @date 20190410
//! @copyright Allegorithmic. All rights reserved.

#pragma once

#include <vector>
#include <utility>
#include <substance/link/framework/callbackstype.h>

namespace Alg
{
namespace Link
{
namespace Framework
{
class Application
{
public:
	//! @brief Virtual destructor
	virtual ~Application() = default;

	//! @brief Virtual method to run preinitialize steps
	virtual void preInit();

	//! @brief Virtual method to run postinitialize steps
	virtual void postInit() = 0;

	//! @brief Virtual method to run preshutdown steps
	virtual void preShutdown();

	//! @brief Virtual method to run postshutdown steps
	virtual void postShutdown() = 0;

protected:
	using CallbackPair = std::pair<substance_link_uuid_t, MessageCallback>;
	std::vector<CallbackPair> mCallbacks;

	//! @brief Register all of the callbacks in mCallbacks
	void registerCallbacks() const;

	//! @brief Unregister all of the callbacks in mCallbacks
	void unregisterCallbacks() const;
};
} // namespace Framework
} // namespace Link
} // namespace Alg
