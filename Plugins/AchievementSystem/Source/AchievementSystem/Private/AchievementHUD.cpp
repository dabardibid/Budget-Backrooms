// Copyright 2021 Sam Carey. All Rights Reserved.

#include "AchievementHUD.h"
#include "UIAchievement.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UAchievementHUD::UAchievementHUD(const FObjectInitializer &ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUIAchievement> UIAchievementClassHelper(TEXT("WidgetBlueprintGeneratedClass'/AchievementSystem/BP_UIAchievement.BP_UIAchievement_C'"));
	UIAchievementClass = UIAchievementClassHelper.Class;
}

void UAchievementHUD::ScheduleAchievementDisplay(FAchievementData Achievement, FAchievementStates State)
{
	if(Active.Num() < Settings->TotalOnScreen && !MovingUp)
	{
		Spawn(Achievement, State);
	}
	else
	{
		FAchievementPackage TempPackage = FAchievementPackage();
		TempPackage.Data = Achievement;
		TempPackage.State = State;
		BackLog.Add(TempPackage);
	}
}

void UAchievementHUD::UpdateHUD(float InDeltaTime)
{
	if (Active.Num() < Settings->TotalOnScreen && !MovingUp)
	{
		if (BackLog.Num() > 0)
		{
			Spawn(BackLog[0].Data, BackLog[0].State);
			BackLog.RemoveAt(0);
		}
	}
	else
	{
		if (MovingUp)
		{
			CurrentTime += InDeltaTime;
			for (UUIAchievement* i : Active)
			{
				UCanvasPanelSlot* CSlot = Cast<UCanvasPanelSlot>(i->Slot);
				CSlot->SetPosition(FVector2D(
					CSlot->GetPosition().X,
					UKismetMathLibrary::MapRangeClamped(CurrentTime, 0, Settings->EntranceAnimationLength, i->StartHeight, i->StartHeight + (Settings->IsBottomOfScreen() ? -Size.Y : Size.Y))
				));
			}
			if (CurrentTime >= Settings->EntranceAnimationLength)
			{
				MovingUp = false;
				CurrentTime = 0;
				for (int i = 0; i < Active.Num(); i++)
				{
					Active[i]->StartHeight = Cast<UCanvasPanelSlot>(Active[i]->Slot)->GetPosition().Y;
				}
			}
		}
	}
}

void UAchievementHUD::Spawn(FAchievementData Achievement, FAchievementStates State)
{
	if(!IsInViewport())
	{
		AddToViewport();
	}
	
	//USoundBase * Sound = Cast<USoundBase>((State.Achieved ? Settings->UnlockSound : Settings->ProgressMadeSound).TryLoad());
	//if(IsValid(Sound))
	//{
	//	UGameplayStatics::PlaySound2D(GetWorld(), Cast<USoundBase>(Sound));
	//}
	
	USoundBase * Sound = State.Achieved ? UnlockSound_Loaded : ProgressMadeSound_Loaded;
	if(IsValid(Sound))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Sound);
	}

	//Select override class if found in the settings menu
	TSubclassOf<UUIAchievement> SelectedUIAchievemntClass = Settings->UiAchievementOverride ? Settings->UiAchievementOverride : UIAchievementClass;
	
	UUIAchievement *Temp = CreateWidget<UUIAchievement>(UGameplayStatics::GetPlayerController(GetWorld(), 0), SelectedUIAchievemntClass);
	Temp->SetValue(Achievement, State, Settings->bShowExactProgress, Settings);
	Active.Add(Temp);
	UCanvasPanelSlot *CSlot = Canvas->AddChildToCanvas(Temp);
	
	Size = FVector2D(Temp->Root->WidthOverride, Temp->Root->HeightOverride);
	SpawnLocation = FVector2D(
		Settings->IsRightOfScreen() ? -Size.X : 0,
		Settings->IsBottomOfScreen() ? 0 : -Size.Y
	);
	
	CSlot->SetSize(Size);
	CSlot->SetAnchors(SpawnAnchors);
	CSlot->SetPosition(SpawnLocation);
	Temp->OnDeath.AddDynamic(this, &UAchievementHUD::OnAchievementDeath);
	Temp->StartDeathTimer(Settings->ExitAnimationLength, Settings->ScreenTime, Settings->ExitAnimation, Settings->IsRightOfScreen());
	Temp->StartHeight = SpawnLocation.Y;
	MovingUp = true;
}

void UAchievementHUD::OnAchievementDeath(UUIAchievement* Achievement)
{
	Achievement->OnDeath.RemoveAll(this);
	Active.Remove(Achievement);
	Achievement->RemoveFromParent();
}

void UAchievementHUD::NativeConstruct()
{
	Super::NativeConstruct();

	Settings = UAchievementSystemSettings::GetSettings();

	SpawnAnchors = FAnchors(
		Settings->IsRightOfScreen() ? 1 : 0,
		Settings->IsBottomOfScreen() ? 1 : 0
	);

	if(Settings->UiAchievementOverride)
	{
		UIAchievementClass = Settings->UiAchievementOverride;
	}

	if(Settings->UnlockSound.IsValid())
	{
		UnlockSound_Loaded = Cast<USoundBase>(Settings->UnlockSound.TryLoad());
	}
	if(Settings->ProgressMadeSound.IsValid())
	{
		ProgressMadeSound_Loaded = Cast<USoundBase>(Settings->ProgressMadeSound.TryLoad());
	}
}

//void UAchievementHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
//{
//	Super::NativeTick(MyGeometry, InDeltaTime);
//	UpdateHUD(InDeltaTime);
//}