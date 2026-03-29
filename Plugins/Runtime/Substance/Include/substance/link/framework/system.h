//! @file system.h
//! @brief Core and system application type
//! @author Galen Helfter - Adobe
//! @date 20190616
//! @copyright Adobe. All rights reserved.

#pragma once

#include <substance/link/framework/application.h>

namespace Alg
{
namespace Link
{
namespace Framework
{
class System : public Application
{
public:
	//! @brief Constructor
	System();

	//! @brief Destructor
	virtual ~System();

	//! @brief Run system application preinitialization steps
	void preInit() override;

	//! @brief Run system application postinitialization
	void postInit() override;

	//! @brief Run system application postshutdown
	void postShutdown() override;

public:
	static const substance_link_uuid_t sConnectionEstablishedId;
	static const substance_link_uuid_t sConnectionClosedId;

	MessageCallback mRecvConnectionEstablished;
	MessageCallback mRecvConnectionClosed;

	size_t mUserData;
};
} // namespace Framework
} // namespace Link
} // namespace Alg
