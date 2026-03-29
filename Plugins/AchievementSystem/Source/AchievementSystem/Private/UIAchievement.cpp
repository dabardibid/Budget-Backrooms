// Copyright 2021 Sam Carey. All Rights Reserved.


#include "UIAchievement.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetTextLibrary.h"


void UUIAchievement::SetValue(FAchievementData Achievement, FAchievementStates State, bool ShowExactProgress, UAchievementSystemSettings* Settings)
{
	Title->SetText(FText::FromString(Achievement.DisplayName));
	Description->SetText(FText::FromString(Achievement.Description));

	if (Achievement.LockOverlay && !State.Achieved)
	{
		OverlayIcon->SetBrushFromTexture(Achievement.LockedIcon);
		Icon->SetBrushFromTexture(Achievement.AchievedIcon);
	}
	else
	{
		OverlayIcon->SetVisibility(ESlateVisibility::Hidden);
		Icon->SetBrushFromTexture(State.Achieved ? Achievement.AchievedIcon : Achievement.LockedIcon);
	}

	if (Achievement.Progress)
	{
		float CurrentProgress = ShowExactProgress ? State.Progress : State.LastProgressUpdate * Achievement.NotificationFrequency;
		float DisplayProgress = State.Achieved ? Achievement.ProgressGoal : CurrentProgress;
		
		FString ps = " " + Achievement.ProgressSuffix;

		FString Progress;
		FString Goal;
		if(!Achievement.IntProgress)
		{
			Progress = UKismetTextLibrary::Conv_FloatToText(DisplayProgress, ERoundingMode::HalfFromZero, false, false, 1, 324, 0, Settings->MaxDecimalPlaces).ToString();
			Goal = UKismetStringLibrary::Conv_FloatToString(Achievement.ProgressGoal);
		}
		else
		{
			Progress = UKismetStringLibrary::Conv_IntToString((int32)(DisplayProgress));
			Goal = UKismetStringLibrary::Conv_IntToString((int32)Achievement.ProgressGoal);
		
		}
	
		FString ProgressText = Progress + ps + " / " + Goal + ps + (State.Achieved ? " " + Settings->AchievedWord : "");

		PercentText->SetText(FText::FromString(ProgressText));
		ProgressBar->SetPercent(DisplayProgress / Achievement.ProgressGoal);
	}
	else
	{
		ProgressBar->SetPercent(State.Achieved ? 1 : 0);
		PercentText->SetText(FText::FromString(State.Achieved ? Settings->AchievedWord : Settings->UnAchievedWord));
	}
}



void UUIAchievement::StartDeathTimer(float ExitAnimationLength, float ScreenTime, EAchievementAnimation ExitAnimation, bool SlideAnimationRight)
{
	FTimerDelegate TimerDel;
	UWidgetAnimation* TempAnimation = nullptr;
	
	switch(ExitAnimation)
	{
		case EAchievementAnimation::SLIDE:
			TempAnimation = (SlideAnimationRight ? SlideRight : SlideLeft);
			break;
		case EAchievementAnimation::FADE:
			TempAnimation = Fade;
			break;
		case EAchievementAnimation::SCALE_DOWN:
			TempAnimation = ScaleDown;
		default: ;
	}
	
	TimerDel.BindUFunction(this, FName("OnDeathEvent"), TempAnimation, ExitAnimationLength);
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, TimerDel, ScreenTime, false);
}

void UUIAchievement::OnDeathEvent(UWidgetAnimation *Animation, float AnimationLength)
{
	if(IsValid(Animation))
	{
		PlayAnimation(Animation, 0, 1, EUMGSequencePlayMode::Forward, 1.0 / AnimationLength, false);
		GetWorld()->GetTimerManager().SetTimer(DeathTimerAnimationComplete, this, &UUIAchievement::OnDeathAnimationComplete, AnimationLength, false);
	}
	else
	{
		OnDeathAnimationComplete();
	}
}

void UUIAchievement::OnDeathAnimationComplete()
{
	OnDeath.Broadcast(this);
}