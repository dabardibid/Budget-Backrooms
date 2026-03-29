//! @file export.h
//! @brief Base application type for live sbsar update and import
//! @author Galen Helfter - Allegorithmic
//! @date 20190411
//! @copyright Allegorithmic. All rights reserved.

#pragma once

#include <substance/link/framework/application.h>

namespace Alg
{
namespace Link
{
namespace Framework
{
class Export : public Application
{
public:
	//! @brief Default constructor
	Export();

	//! @brief Destructor
	virtual ~Export();

	//! @brief Run live export preinitialization steps
	void preInit() override;

	//! @brief Run live export postinitialization
	void postInit() override;

	//! @brief Run live export postshutdown
	void postShutdown() override;

	//! @brief Write an export message to a given connection
	void writeLoadSbsar(unsigned int context, const char* message);

public:
	static const substance_link_uuid_t sLoadSbsarId;
	static const substance_link_uuid_t sUpdateSbsarId;

	MessageCallback mRecvLoadSbsar;
	MessageCallback mRecvUpdateSbsar;

	size_t mUserData;
};
} // namespace Framework
} // namespace Link
} // namespace Alg
