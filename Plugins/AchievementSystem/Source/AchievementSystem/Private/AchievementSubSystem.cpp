// Copyright 2021 Sam Carey. All Rights Reserved.

#include "AchievementSubSystem.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

UAchievementSubSystem::UAchievementSubSystem()
{
	//Load Blueprint versions of UI elements
	static ConstructorHelpers::FClassFinder<UAchievementHUD> HUDClassHelper(TEXT("WidgetBlueprintGeneratedClass'/AchievementSystem/BP_AchievementHUD.BP_AchievementHUD_C'"));
	AchievementHUDClass = HUDClassHelper.Class;
}

void UAchievementSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Settings = UAchievementSystemSettings::GetSettings();

	if(UGameplayStatics::DoesSaveGameExist(Settings->SaveGameName, 0))
	{
		SaveGame = Cast<UAchievementSaveGame>(UGameplayStatics::LoadGameFromSlot(Settings->SaveGameName, 0));
	}
	else
	{
		SaveGame = Cast<UAchievementSaveGame>(UGameplayStatics::CreateSaveGameObject(UAchievementSaveGame::StaticClass()));
	}
	SaveGame->States.SetNum(Settings->Achievements.Num(), true);
	UGameplayStatics::SaveGameToSlot(SaveGame, Settings->SaveGameName, 0);
}

bool UAchievementSubSystem::AchievementExists(FName Key)
{
	AchievementStorageData Temp = Settings->GetAchievementLocation(Key);
	return Temp.Index != -1;
}

int UAchievementSubSystem::GetAchievedCount()
{
	return SaveGame->TotalUnlocked;
}

float UAchievementSubSystem::GetAchievedPercentage()
{
	if (SaveGame->States.Num() == 0)
	{
		return 0;
	}
	return (float)GetAchievedCount() / SaveGame->States.Num() * 100;
}

void UAchievementSubSystem::GetAchievementState(FName Key, bool &FoundAchievement, FAchievementData &Data, FAchievementStates& State)
{
	FoundAchievement = false;
	AchievementStorageData i = Settings->GetAchievementLocation(Key);
	if (i.Index == -1)
		return;

	const FAchievementData Current = i.Data;
	int32 index = i.Index;
	
	State = SaveGame->States[i.Index];
	Data = Current;
	FoundAchievement = true;
}


void UAchievementSubSystem::Unlock(FName Key)
{
	AchievementStorageData i = Settings->GetAchievementLocation(Key);
	if(i.Index == -1)
		return;
	
	const FAchievementData Current = i.Data;
	int32 index = i.Index;
	
	if (!SaveGame->States[index].Achieved)
	{
		SaveGame->TotalUnlocked += 1;
		SaveGame->States[index].Progress = Current.ProgressGoal;
		SaveGame->States[index].Achieved = true;
		OnAchievementUnlock.Broadcast(Key);
		DisplayProgress(Current, index);
		TryAutoSaveStates();

		if(Settings->bUseCompletionistAchievement && SaveGame->TotalUnlocked == Settings->Achievements.Num() - 1)
		{
			Unlock(Settings->CompletionistAchievementKey);
		}
	}
}

void UAchievementSubSystem::SetAchievementProgress(FName Key, float Progress)
{
	AchievementStorageData i = Settings->GetAchievementLocation(Key);
	if (i.Index == -1)
		return;
	const FAchievementData Current = i.Data;
	int32 index = i.Index;
	
	if (Current.Progress)
	{
		if (Progress >= Current.ProgressGoal)
		{
			Unlock(Key);
		}
		else
		{
			SaveGame->States[index].Progress = Progress;
			OnAchievementProgressUpdate.Broadcast(Key, SaveGame->States[index].Progress, Progress);
			DisplayProgress(Current, index);
		}
	}
	TryAutoSaveStates();
}

void UAchievementSubSystem::AddAchievementProgress(FName Key, float Progress)
{
	AchievementStorageData i = Settings->GetAchievementLocation(Key);
	if (i.Index == -1)
		return;
	
	const FAchievementData Current = i.Data;
	int32 index = i.Index;

	if (Current.Progress)
	{
		if (SaveGame->States[index].Progress + Progress >= Current.ProgressGoal)
		{
			Unlock(Key);
		}
		else
		{
			SaveGame->States[index].Progress += Progress;
			OnAchievementProgressUpdate.Broadcast(Key, SaveGame->States[index].Progress, Progress);
			DisplayProgress(Current, index);
			TryAutoSaveStates();
		}
	}
}

void UAchievementSubSystem::SaveAchievementState()
{
	UGameplayStatics::SaveGameToSlot(SaveGame, Settings->SaveGameName, 0);
}

void UAchievementSubSystem::ResetAchievementStates()
{
	UGameplayStatics::DeleteGameInSlot(Settings->SaveGameName, 0);
	SaveGame = Cast<UAchievementSaveGame>(UGameplayStatics::CreateSaveGameObject(UAchievementSaveGame::StaticClass()));
	SaveGame->States.SetNum(Settings->Achievements.Num(), false);
	UGameplayStatics::SaveGameToSlot(SaveGame, Settings->SaveGameName, 0);
}

void UAchievementSubSystem::Tick(float DeltaTime)
{
	if (IsValid(Screen))
	{
		//Add back into viewport on level load
		if (!Screen->IsInViewport())
		{
			Screen->AddToViewport();
		}
		Screen->UpdateHUD(DeltaTime);
	}
}

void UAchievementSubSystem::TryAutoSaveStates()
{
	if(Settings->bAutoSave)
	{
		SaveAchievementState();
	}
}

void UAchievementSubSystem::DisplayProgress(FAchievementData Achievement, int Index)
{
	if((Settings->bShowAchievementUnlocks && SaveGame->States[Index].Achieved) || (Settings->bShowAchievementProgress && !SaveGame->States[Index].Achieved))
	{
		if (!Achievement.Spoiler || SaveGame->States[Index].Achieved)
		{
			UAchievementHUD* HUD = GetHUD();
			if (Achievement.Progress && SaveGame->States[Index].Progress < Achievement.ProgressGoal)
			{
				if(Achievement.NotificationFrequency > 0)
				{
					int32 Steps = (int32)Achievement.ProgressGoal / (int32)Achievement.NotificationFrequency;

					//Loop from biggest notification bracket to the smallest
					for (int32 i = Steps; i > SaveGame->States[Index].LastProgressUpdate; i--)
					{
						if (SaveGame->States[Index].Progress >= Achievement.NotificationFrequency * i)
						{
							SaveGame->States[Index].LastProgressUpdate = i;
							HUD->ScheduleAchievementDisplay(Achievement, SaveGame->States[Index]);
							return;
						}
					}
				}
			}
			else
			{
				HUD->ScheduleAchievementDisplay(Achievement, SaveGame->States[Index]);
			}
		}
	}
}

UAchievementHUD* UAchievementSubSystem::GetHUD()
{
	if(IsValid(Screen))
	{
		//Add back into viewport on level load
		if(!Screen->IsInViewport())
		{
			Screen->AddToViewport();
		}
		return Screen;
	}
	Screen = CreateWidget<UAchievementHUD>(UGameplayStatics::GetPlayerController(GetWorld(), 0), AchievementHUDClass);
	Screen->AddToViewport();
	return Screen;
}