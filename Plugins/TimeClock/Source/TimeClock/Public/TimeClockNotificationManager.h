// Copyright Lambda Works, Samuel Metters 2019. All rights reserved.

#pragma once

#include "CoreMinimal.h"

class SNotificationItem;

/**
 * FSimpleNotificationInfo
 */
class TIMECLOCK_API FTimeClockNotificationManager
{
public:
	FTimeClockNotificationManager() {}
	~FTimeClockNotificationManager();

	// Allows this object to be referenced anywhere.
	static TSharedRef<FTimeClockNotificationManager> Get();

	// Creates and spawn an Editor notification with a button to dismiss it.
	void DispatchNotification(FString Message, float Duration, bool PlaySound);

	// Dismisses the notification with the corresponding text.
	void CancelNotification(FString NotificationMessage);

private:
	// Reference to the instance of this manager.
	static TSharedPtr<FTimeClockNotificationManager> Instance;

	// Map of all spawned notifications.
	TMap<FString, TSharedPtr<SNotificationItem>> SpawnedNotifications;
};
