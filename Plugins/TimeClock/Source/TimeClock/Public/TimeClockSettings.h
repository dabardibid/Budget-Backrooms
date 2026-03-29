// Copyright Lambda Works, Samuel Metters 2019. All rights reserved.
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TimeClockAlarm.h"
#include "TimeClockSettings.generated.h"

/**
 * Implements settings for the TimeClock plugin.
 */
UCLASS(config = TimeClock, defaultconfig)
class TIMECLOCK_API UTimeClockSettings : public UObject
{
	GENERATED_UCLASS_BODY()

// List of all alarms saved for this project. You can export/import these to any other project like you would other settings (see buttons top right of the Project Settings window).
UPROPERTY(EditAnywhere, config, Category = Alarms)
TArray<FTimeClockAlarmData> Alarms;

};
