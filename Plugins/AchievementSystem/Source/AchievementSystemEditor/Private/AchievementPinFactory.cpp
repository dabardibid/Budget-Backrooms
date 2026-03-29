// Copyright 2021 Sam Carey. All Rights Reserved.

#include "AchievementPinFactory.h"
#include "EdGraphSchema_K2.h"
#include "SGraphPin.h"
#include "GraphPinAchievementNames.h"
#include "AchievementSubSystem.h"

TSharedPtr<SGraphPin> FAchievementPinFactory::CreatePin(UEdGraphPin* InPin) const
{
	const FString ToolTipString = FString("Target is Achievement Sub System");
	const FString KeyString = FString("Key");

	if(InPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Name)
	{
		UEdGraphNode *GraphNode = InPin->GetOuter();

		FString Cur = GraphNode->GetTooltipText().ToString();
		if(IsValid(GraphNode))
		{
			//If node is part of subsystem and has the pin name of Key
			if(Cur.EndsWith(ToolTipString) && InPin->GetDisplayName().ToString().Equals(KeyString))
			{
				return SNew(SGraphPinAchievementNames, InPin);
			}
		}
		
	}
	return  nullptr;
}