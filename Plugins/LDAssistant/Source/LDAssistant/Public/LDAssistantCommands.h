// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LDAssistantStyle.h"

class FLDAssistantCommands : public TCommands<FLDAssistantCommands>
{
public:

	FLDAssistantCommands()
		: TCommands<FLDAssistantCommands>(TEXT("LDAssistant"), NSLOCTEXT("Contexts", "LDAssistant", "LDAssistant Plugin"), NAME_None, FLDAssistantStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
