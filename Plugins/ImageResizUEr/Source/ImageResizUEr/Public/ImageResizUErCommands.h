// ImageResizUEr
// Copyright 2015-2022 Turfster / NT Entertainment
// All Rights Reserved.


#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ImageResizUErStyle.h"

class FImageResizUErCommands : public TCommands<FImageResizUErCommands>
{
public:

	FImageResizUErCommands()
		: TCommands<FImageResizUErCommands>(TEXT("ImageResizUEr"), NSLOCTEXT("Contexts", "ImageResizUEr", "ImageResizUEr Plugin"), NAME_None, FImageResizUErStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};