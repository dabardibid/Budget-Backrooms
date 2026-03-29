// Copyright 2021 Sam Carey. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AchievementStates.generated.h"

USTRUCT(BlueprintType, meta = (ToolTip = "Stores / Saves the state of an achievement"))
struct FAchievementStates
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Current progress"), Category="Achievement States")
		float Progress = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Last progress notification bracket"), Category = "Achievement States")
		int LastProgressUpdate = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ToolTip = "Has this been unlocked"), Category = "Achievement States")
		bool Achieved = false;
};