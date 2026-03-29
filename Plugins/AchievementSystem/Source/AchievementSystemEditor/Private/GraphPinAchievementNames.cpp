// Copyright 2021 Sam Carey. All Rights Reserved.


#include "GraphPinAchievementNames.h"
#include "AchievementSystemSettings.h"

SGraphPinAchievementNames::SGraphPinAchievementNames()
{
}

void SGraphPinAchievementNames::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj)
{
	RegenList();
	SGraphPinNameList::Construct(SGraphPinNameList::FArguments(), InGraphPinObj, NameList);

	//FString PreviousSelection = GraphPinObj->GetDefaultAsString();
	//FString a = PreviousSelection;
}

void SGraphPinAchievementNames::RegenList()
{
	NameList.Empty();
	NameList = UAchievementSystemSettings::GetSettings()->GetComboBoxNames();
}