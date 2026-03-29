// Copyright Lambda Works, Samuel Metters 2019. All rights reserved.
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "TimeClock.h"
#include "TimeClockCore.h"
#include "TimeClockAlarm.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TimeClockBPLibrary.generated.h"

class UTimeClockCore;

UCLASS()
class TIMECLOCK_API UTimeClockBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	//The Time Clock Object is responsible for anything the TimeClock plugin can do.
	UFUNCTION(BlueprintPure, meta = (Keywords = "Time Clock"), Category = "Time Clock")
		static UTimeClockCore* GetTimeClockObject();

	//The Time Clock Object is responsible for anything the TimeClock plugin can do.
	UFUNCTION(BlueprintPure, meta = (Keywords = "Time Clock Alarm"), Category = "Time Clock")
		static UTimeClockAlarm* GetTimeClockAlarmObject();

	// Creates and spawns an editor notification.
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock"), Category = "Time Clock")
		static void AddEditorNotification(FString Message, float Duration, bool PlaySound);
};
