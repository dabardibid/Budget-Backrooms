// Copyright 2021 Sam Carey. All Rights Reserved.


#include "AchievementSystemSettings.h"

UAchievementSystemSettings::UAchievementSystemSettings(const FObjectInitializer& ObjectInitializer)
{
	if(!UnlockSound.IsValid())
	{
		UnlockSound = FString("/AchievementSystem/Example/Sound/A_Default_Unlock_Cue.A_Default_Unlock_Cue");
	}
	if(!ProgressMadeSound.IsValid())
	{
		ProgressMadeSound = FString("/AchievementSystem/Example/Sound/A_Default_Progress_Cue.A_Default_Progress_Cue");
	}
}

AchievementStorageData UAchievementSystemSettings::GetAchievementLocation(FName Key)
{
	AchievementStorageData location = AchievementStorageData();
	for(int i = 0; i < Achievements.Num(); i++)
	{
		if(Achievements[i].Key == Key)
		{
			location.Data = Achievements[i];
			location.Index = i;
		}
	}
	return location;
}

bool UAchievementSystemSettings::IsRightOfScreen() const
{
	return ScreenLocation == EAchievementSceenLocation::TOP_RIGHT || ScreenLocation == EAchievementSceenLocation::BOTTOM_RIGHT;
}

bool UAchievementSystemSettings::IsBottomOfScreen() const
{
	return ScreenLocation == EAchievementSceenLocation::BOTTOM_LEFT || ScreenLocation == EAchievementSceenLocation::BOTTOM_RIGHT;
}

UAchievementSystemSettings* UAchievementSystemSettings::GetSettings()
{
	return GetMutableDefault<UAchievementSystemSettings>();
}

TArray<TSharedPtr<FName>> UAchievementSystemSettings::GetComboBoxNames()
{
	TArray<TSharedPtr<FName>> NameList;
	for(FAchievementData i : Achievements)
	{
		if(CompletionistAchievementKey != i.Key)
		{
			TSharedPtr<FName> NameKey = MakeShareable(new FName(i.Key));
			NameList.Add(NameKey);
		}
	}
	return NameList;
}