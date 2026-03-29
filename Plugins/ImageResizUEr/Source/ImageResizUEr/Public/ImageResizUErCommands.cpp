// ImageResizUEr
// Copyright 2015-2022 Turfster / NT Entertainment
// All Rights Reserved.



#include "ImageResizUErCommands.h"

#define LOCTEXT_NAMESPACE "FImageResizUErModule"

void FImageResizUErCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "ImageResizUEr", "Bring up ImageResizUEr window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
