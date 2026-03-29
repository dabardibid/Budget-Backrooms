// Copyright 2021 Sam Carey. All Rights Reserved.

#include "AchievementProgressDetails.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetTextLibrary.h"
#include "Kismet/KismetStringLibrary.h"

void UAchievementProgressDetails::UpdateDetails()
{
	Title->SetText(FText::FromString(TitleText));
	AchievementProgressBar->SetFillColorAndOpacity(ProgressBarColour);
	Title->SetVisibility(TitleText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	int32 TotalAchieved = SubSystem->GetAchievedCount();
	int32 Total = Settings->Achievements.Num();
	float Progress = (float)TotalAchieved / (float)Total;

	FString ProgressString = UKismetTextLibrary::Conv_FloatToText(Progress * 100, ERoundingMode::HalfFromZero, false, false, 1, 324, 0, TotalProgressMaxDecimalPlaces).ToString();

	AchievementProgressBar->SetPercent(Progress);
	FString TotalText =
		UKismetStringLibrary::Conv_IntToString(TotalAchieved) + " / " +
		UKismetStringLibrary::Conv_IntToString(Total) + " (" +
		ProgressString + "%)";

	ProgressText->SetText(FText::FromString(TotalText));
}

void UAchievementProgressDetails::NativePreConstruct()
{
	if(IsValid(Title))
	{
		Title->SetText(FText::FromString(TitleText));
		Title->SetVisibility(TitleText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
	if (IsValid(AchievementProgressBar))
	{
		AchievementProgressBar->SetFillColorAndOpacity(ProgressBarColour);
	}
}

void UAchievementProgressDetails::NativeConstruct()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	SubSystem = GameInstance->GetSubsystem<UAchievementSubSystem>();
	Settings = UAchievementSystemSettings::GetSettings();
	UpdateDetails();
}
