// Copyright 2021 Sam Carey. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Sound/SoundBase.h"
#include "AchievementData.h"
#include "AchievementSystemSettings.generated.h"


class UUIAchievement;
class UAchievementHUD;

struct AchievementStorageData
{
	FAchievementData Data;
	int Index = -1;
};

UENUM(BlueprintType)
enum class EAchievementSceenLocation : uint8
{
	TOP_LEFT 		UMETA(DisplayName = "Top Left"),
	TOP_RIGHT		UMETA(DisplayName = "Top Right"),
	BOTTOM_LEFT		UMETA(DisplayName = "Bottom Left"),
	BOTTOM_RIGHT	UMETA(DisplayName = "Bottom Right")
};

UENUM(BlueprintType)
enum class EAchievementAnimation : uint8
{
	FADE 			UMETA(DisplayName = "Fade"),
	DISAPPEAR		UMETA(DisplayName = "Disappear"),
	SLIDE			UMETA(DisplayName = "Slide"),
	SCALE_DOWN		UMETA(DisplayName = "Scale Down")
};

/**
 * 
 */
UCLASS(config = Game, defaultconfig, Category = "Holds all settings for the achievement system")
class ACHIEVEMENTSYSTEM_API UAchievementSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UAchievementSystemSettings(const FObjectInitializer& ObjectInitializer);

	/**List of all achievement which can be unlocked by the player */
	UPROPERTY(config, EditAnywhere, Category = "Achievements", meta=(TitleProperty = "Key"))
		TArray<FAchievementData> Achievements;

	/**If true, CompletionistAchievementKey will be unlocked when all other achievement have be gained*/
	UPROPERTY(config, EditAnywhere, Category = "Achievements")
		bool bUseCompletionistAchievement = false;

	/**The key of the achievement to be unlocked when all others have be gained*/
	UPROPERTY(config, EditAnywhere, Category = "Achievements")
		FName CompletionistAchievementKey = FName();

	/**If true, achievement unlock notifications will be displayed on the players screen. */
	UPROPERTY(config, EditAnywhere, Category = "Achievement Unlocks")
		bool bShowAchievementUnlocks = true;
	/**If true, achievement progress update notifications will be displayed on the players screen. */
	UPROPERTY(config, EditAnywhere, Category = "Achievement Unlocks")
		bool bShowAchievementProgress = true;
	/**Define the corner of the screen that progress and unlock notifications will be displayed. */
	UPROPERTY(config, EditAnywhere, Category = "Achievement Unlocks")
		EAchievementSceenLocation ScreenLocation = EAchievementSceenLocation::BOTTOM_RIGHT;
	/**The total number of achievement notifications which can be on the screen at any one time.*/
	UPROPERTY(config, EditAnywhere, Category = "Achievement Unlocks")
		int TotalOnScreen = 3;
	/**The number of seconds an achievement will display on the screen once unlocked or progress is made.*/
	UPROPERTY(config, EditAnywhere, Category = "Achievement Unlocks")
		float ScreenTime = 3;
	/**If true, progress notifications will display their exact progress, if false it will show the closest bracket.*/
	UPROPERTY(config, EditAnywhere, Category = "Achievement Unlocks")
		bool bShowExactProgress = false;
	/**The max number of decimal places which can be displayed on a UI Achievement*/
	UPROPERTY(config, EditAnywhere, Category = "Achievement Unlocks")
		int MaxDecimalPlaces = 2;
	/**If true, the state of all achievements will be saved without any user input. If false, achievement states must be saved manually by calling  SaveAchievementState().*/
	UPROPERTY(config, EditAnywhere, Category = "Achievement Unlocks")
		bool bAutoSave = true;
	/**The name of the save game slot used to keep track of achievement states.*/
	UPROPERTY(config, EditAnywhere, Category = "Achievement Unlocks")
		FString SaveGameName = "SaveGame_Achievements";

	/**The animation that is played when notifications are removed from the screen. This can either be Fade, Disappear, Slide or Scale Down.*/
	UPROPERTY(config, EditAnywhere, Category = "Achievement Animation")
		EAchievementAnimation ExitAnimation = EAchievementAnimation::SLIDE;
	/**The number of seconds an achievement notification takes to leave the screen.*/
	UPROPERTY(config, EditAnywhere, Category = "Achievement Animation")
		float ExitAnimationLength = 0.5;
	/**The number of seconds an achievement notification takes to enter the screen.*/
	UPROPERTY(config, EditAnywhere, Category = "Achievement Animation")
		float EntranceAnimationLength = 0.2;
	
	/**The sound that plays when an achievement is unlocked is displayed to a user. Only plays if ShowAchievementUnlocks is true.*/
	UPROPERTY(config, EditAnywhere, Category = "Sound", meta = (AllowedClasses = "SoundBase"))
		FStringAssetReference UnlockSound;
	/**The sound that plays when a progress update is displayed to a user. Only plays if ShowAchievementProgress is true.*/
	UPROPERTY(config, EditAnywhere, Category = "Sound", meta = (AllowedClasses = "SoundBase"))
		FStringAssetReference ProgressMadeSound;

	/**Use this achievement ui to display progress and unlock updates to the players screen*/
	UPROPERTY(config, EditAnywhere, Category = "UI Achievement Indicator")
		TSubclassOf<UUIAchievement> UiAchievementOverride;
	/**Appends to the progress bar when an achievement has not been achieved*/
	UPROPERTY(config, EditAnywhere, Category = "UI Achievement Indicator")
		FString AchievedWord = FString("(Achieved)");
	/**Appends to the progress bar when an achievement is achieved*/
	UPROPERTY(config, EditAnywhere, Category = "UI Achievement Indicator")
		FString UnAchievedWord = FString("(Locked)");
	
	AchievementStorageData GetAchievementLocation(FName Key);
	bool IsRightOfScreen() const;
	bool IsBottomOfScreen() const;

	static UAchievementSystemSettings* GetSettings();
	TArray<TSharedPtr<FName>> GetComboBoxNames();
};