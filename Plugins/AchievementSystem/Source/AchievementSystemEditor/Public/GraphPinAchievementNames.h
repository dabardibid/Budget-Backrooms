// Copyright 2021 Sam Carey. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPinNameList.h"
#include "Widgets/DeclarativeSyntaxSupport.h"


class ACHIEVEMENTSYSTEMEDITOR_API SGraphPinAchievementNames : public SGraphPinNameList
{
public:
	SLATE_BEGIN_ARGS(SGraphPinAchievementNames) {}
	SLATE_END_ARGS()

	SGraphPinAchievementNames();
	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj);

protected:
	void RegenList ();
};
