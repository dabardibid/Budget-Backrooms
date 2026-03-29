// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "LDAssistantCommands.h"

#define LOCTEXT_NAMESPACE "FLDAssistantModule"

void FLDAssistantCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "LDAssistant", "Execute LDAssistant action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
