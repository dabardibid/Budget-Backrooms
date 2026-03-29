// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: ISubstanceCore.h

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

/** The public interface of the SubstanceCore module */
class ISubstanceCore : public IModuleInterface
{
public:
	/** Returns the Maximum Output size for Substance Texture 2D */
	virtual uint32 GetMaxOutputTextureSize() const = 0;
};
