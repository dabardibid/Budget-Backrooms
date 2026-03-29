// Copyright Lambda Works, Samuel Metters 2019. All rights reserved.


#include "TimeClockNotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "FTimeClockNotificationManager"

TSharedPtr<FTimeClockNotificationManager> FTimeClockNotificationManager::Instance = nullptr;
TSharedRef<FTimeClockNotificationManager> FTimeClockNotificationManager::Get()
{
	if (!Instance.IsValid())
	{
		Instance = MakeShareable(new FTimeClockNotificationManager());
	}
	return Instance.ToSharedRef();
}

void FTimeClockNotificationManager::DispatchNotification(FString Message, float Duration, bool PlaySound)
{
	// These variables are the ones will be storing to allow for a delayed dismissal
	FString NotificationMessage = Message + " (Time Clock Alarm)";
	TSharedPtr<SNotificationItem> NotificationItem;

	// Initialise the notification object with a button (and its delegate to dismiss the notification).
	FNotificationInfo Info(FText::FromString(NotificationMessage));
	Info.bFireAndForget = false;
	Info.FadeInDuration = 0.5f;
	Info.FadeOutDuration = 0.5f;
	Info.ExpireDuration = Duration;
	Info.bUseThrobber = false;
	Info.bUseSuccessFailIcons = true;
	Info.bUseLargeFont = true;
	Info.bAllowThrottleWhenFrameRateIsLow = false;
	Info.ButtonDetails.Add(
		FNotificationButtonInfo(
			FText::FromString("Dimiss"),
			FText(),
			FSimpleDelegate::CreateRaw(this, &FTimeClockNotificationManager::CancelNotification, NotificationMessage),
			SNotificationItem::CS_None));

	// Spawns the notification and save the pointer to that notification for later.
	NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
	SpawnedNotifications.Add(NotificationMessage, NotificationItem);

	// Optionnally play a sound when the notification goes through
	if (PlaySound)
	{
		GEditor->PlayEditorSound(TEXT("/Engine/EditorSounds/Notifications/CompileStart_Cue.CompileStart_Cue"));
	}
}

void FTimeClockNotificationManager::CancelNotification(FString NotificationMessage)
{
	// Check if we have stored a notification with that message.
	if (SpawnedNotifications.Contains(NotificationMessage))
	{
		// Dissmiss the notification
		SpawnedNotifications.Find(NotificationMessage)->Get()->SetExpireDuration(0.0f);
		SpawnedNotifications.Find(NotificationMessage)->Get()->ExpireAndFadeout();
		SpawnedNotifications.Find(NotificationMessage)->Reset();

		SpawnedNotifications.Remove(NotificationMessage);
	}
}

FTimeClockNotificationManager::~FTimeClockNotificationManager()
{
	if (Instance.IsValid())
	{
		Instance.Reset();
	}
}
#undef LOCTEXT_NAMESPACE