// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceStyle.h

#pragma once
#include "Styling/ISlateStyle.h"
#include "Styling/SlateStyle.h"

class FSubstanceStyle
{
public:
	/** Initializes the class */
	static void Initialize();

	/** Cleans up the class */
	static void Shutdown();

	/** Return the name of the Style Set */
	static FName GetStyleSetName();

	/** Returns the shared pointer to the slate style set */
	static TSharedPtr<class ISlateStyle> Get()
	{
		return StyleSet;
	}

private:
	/** Returns a complete file path */
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);

	/** Pointer to the style set */
	static TSharedPtr<class FSlateStyleSet> StyleSet;
};
