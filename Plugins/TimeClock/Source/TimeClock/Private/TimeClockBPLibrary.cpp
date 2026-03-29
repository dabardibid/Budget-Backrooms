// Copyright Lambda Works, Samuel Metters 2019. All rights reserved.
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "TimeClockBPLibrary.h"
#include "TimeClockCore.h"
#include "TimeClockAlarm.h"
#include "TimeClock.h"
#include "TimeClockNotificationManager.h"

UTimeClockCore* UTimeClockBPLibrary::GetTimeClockObject()
{
	UTimeClockCore* ReturnObject;

	FModuleManager::GetModuleChecked<FTimeClockModule>("TimeClock").GetTimeClockCoreObject(ReturnObject);

	return ReturnObject;
};

UTimeClockAlarm* UTimeClockBPLibrary::GetTimeClockAlarmObject()
{
	UTimeClockAlarm* ReturnObject;

	FModuleManager::GetModuleChecked<FTimeClockModule>("TimeClock").GetTimeClockAlarmObject(ReturnObject);

	return ReturnObject;
}

void UTimeClockBPLibrary::AddEditorNotification(FString Message, float Duration, bool PlaySound)
{
	FTimeClockNotificationManager::Get()->DispatchNotification(Message, Duration, PlaySound);
	
}

