// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceCorePrivatePCH.h

#pragma once
#include "UObject/UObjectGlobals.h"

//NOTE:: This is added to prevent including framework classes outside of our module. The allows C++ projects
//to use our core classes without breaking encapsulation for our framework headers.
#define SUBSTANCE_FRAMEWORK_INCLUDED 1

/** This flag is here to toggle on the Substance Stat System
	Note - Using stat system may cause crash: Please see FThreadStatsPool::NUM_ELEMENTS_IN_POOL */
#define SUBSTANCE_MEMORY_STAT 1

#if SUBSTANCE_MEMORY_STAT
DECLARE_MEMORY_STAT_EXTERN(TEXT("Substance Image Memory"), STAT_SubstanceImageMemory, STATGROUP_SceneMemory, SUBSTANCECORE_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Substance Cache Memory"), STAT_SubstanceCacheMemory, STATGROUP_SceneMemory, SUBSTANCECORE_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Substance Engine Memory"), STAT_SubstanceEngineMemory, STATGROUP_SceneMemory, SUBSTANCECORE_API);
#endif

#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "ISubstanceCore.h"
#include "SubstanceCoreCustomVersion.h"

#include <substance/handle.h>

#include "Logging/LogMacros.h"

#include "Runtime/CoreUObject/Public/UObject/NoExportTypes.h"
#include "Runtime/Engine/Public/TextureResource.h"

DEFINE_LOG_CATEGORY_STATIC(LogSubstanceCore, Warning, All);
