// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceCoreCustomVersion.h

#pragma once

#include "Runtime/Core/Public/Serialization/CustomVersion.h"

// Custom serialization version for all packages containing Substance asset types
struct FSubstanceCoreCustomVersion
{
	enum Type
	{
		// Before any version changes were made in the plugin
		BeforeCustomVersionWasAdded = 0,

		// Fixed freezing (non dynamic graphs) system for Substance Graph Instance
		FixedGraphFreeze = 1,
		FrameworkRefactor = 2,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	// The GUID for this custom version number
	const static FGuid GUID;

private:
	FSubstanceCoreCustomVersion() {}
};
