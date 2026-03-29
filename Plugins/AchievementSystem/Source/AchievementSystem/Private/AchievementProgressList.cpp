// Copyright 2021 Sam Carey. All Rights Reserved.

#include "AchievementProgressList.h"
#include "UObject/ConstructorHelpers.h"
#include "UIAchievement.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/GameplayStatics.h"

UAchievementProgressList::UAchievementProgressList(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUIAchievement> UIAchievementClassHelper(TEXT("WidgetBlueprintGeneratedClass'/AchievementSystem/BP_UIAchievement.BP_UIAchievement_C'"));
	UIAchievementClass = UIAchievementClassHelper.Class;
}

void UAchievementProgressList::UpdateList()
{
	TArray<FAchievementStates> States = SubSystem->SaveGame->States;
	TArray<FAchievementData> Data = Settings->Achievements;
	int32 InList = 0;
	
	ListView->ClearChildren();
	for(int i = 0; i < Data.Num(); i++)
	{
		if(	Filter == EAchievementProgressFilter::ANY || 
			Filter == EAchievementProgressFilter::ACHIEVED && States[i].Achieved ||
			Filter == EAchievementProgressFilter::UNACHIEVED && !States[i].Achieved
		)
		{
			if(!Data[i].Spoiler || States[i].Achieved)
			{
				TSubclassOf<UUIAchievement> SelectedUIAchievementClass = Settings->UiAchievementOverride ? Settings->UiAchievementOverride : UIAchievementClass;
				UUIAchievement* Temp = CreateWidget<UUIAchievement>(UGameplayStatics::GetPlayerController(GetWorld(), 0), SelectedUIAchievementClass);
				Temp->SetValue(Settings->Achievements[i], SubSystem->SaveGame->States[i], true, Settings);
				ListView->AddChild(Temp);
			}
			else
			{
				InList++;
			}
		}
	}

	HiddenText->SetVisibility(bShowHiddenAchievementCount ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if(bShowHiddenAchievementCount)
	{
		FString Text = UKismetStringLibrary::Conv_IntToString(InList) + " Hidden Achievement" + (InList > 1? "s" : "");
		HiddenText->SetText(FText::FromString(Text));
	}
}

void UAchievementProgressList::NativePreConstruct()
{
	if(!IsValid(HiddenText))
		return;

	HiddenText->SetVisibility(bShowHiddenAchievementCount ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UAchievementProgressList::NativeConstruct()
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
	SubSystem = GameInstance->GetSubsystem<UAchievementSubSystem>();
	Settings = UAchievementSystemSettings::GetSettings();
	UpdateList();
}