//! @file livelink.h
//! @brief Defines the interface for a livelink application
//! @author Galen Helfter - Allegorithmic
//! @date 20190411
//! @copyright Allegorithmic - All rights reserved.

#pragma once

#include <substance/link/framework/application.h>

namespace Alg
{
namespace Link
{
namespace Framework
{
class Livelink : public Application
{
public:
	//! @brief Destructor
	~Livelink();

	//! @brief Run livelink preinitialization steps
	void preInit() override;

	//! @brief Run livelink postinitialization
	void postInit() override;

	// @brief Run livelink postshutdown
	void postShutdown() override;

	MessageCallback mUpdateImage;
};
} // namespace Framework
} // namespace Link
} // namespace Alg
