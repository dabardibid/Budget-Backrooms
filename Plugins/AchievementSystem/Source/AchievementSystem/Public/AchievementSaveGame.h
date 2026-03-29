// Copyright 2021 Sam Carey. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AchievementStates.h"
#include "AchievementSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class ACHIEVEMENTSYSTEM_API UAchievementSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FAchievementStates> States;
	UPROPERTY()
	int32 TotalUnlocked = 0;
};
