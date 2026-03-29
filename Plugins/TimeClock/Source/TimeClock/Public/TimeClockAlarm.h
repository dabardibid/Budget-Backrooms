// Copyright Lambda Works, Samuel Metters 2019. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TimeClockAlarm.generated.h"


USTRUCT(BlueprintType)
struct TIMECLOCK_API FTimeClockAlarmData
{
	GENERATED_BODY()

		// Name of the project these stats were taken from.
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Time Clock Alarm")
		FString AlarmText;
		// The data at which the stats started being recorded.
	UPROPERTY(config, EditAnywhere, BlueprintReadWrite, Category = "Time Clock Alarm")
		FDateTime AlarmTime;

	FTimeClockAlarmData()
	{
		AlarmText = "Alarm not initialised.";
	}

	FTimeClockAlarmData(FString InAlarmText, FDateTime InAlarmTime)
	{
		AlarmText = InAlarmText;
		AlarmTime = InAlarmTime;
	}
	inline bool operator==(const FTimeClockAlarmData& other) const
	{
		return AlarmText == other.AlarmText && AlarmTime == other.AlarmTime;
	}
};


UCLASS(BlueprintType, Blueprintable)
class TIMECLOCK_API UTimeClockAlarm : public UObject
{
	GENERATED_BODY()

		UTimeClockAlarm();
	~UTimeClockAlarm();

public:

	// How long should the notification stay for.
	float AlarmDuration = 5.0f;

	// How often should we check for alarms to trigger.
	float AlarmTimerRate = 60.0f;



	/* ==== Alarm Timer ==== */

	FTimerHandle AlarmTimerHandle;
	FDelegateHandle StartupDelegate;

	// Prepares the alarm object, and ensures the timer starts when the editor has loaded.
	UFUNCTION()
	void InitialiseTimeClockAlarm();

	// Starts the timer which regularly checks for any alarms to trigger.
	UFUNCTION()
		void StartAlarmTimer();

	// The function called to chekc for any alarms to trigger.
	UFUNCTION()
		void TimeClockAlarmTick();

	// Triggers the alarm notification from FTimeClockAlarmData struct.
	UFUNCTION()
		void TriggerAlarm(FTimeClockAlarmData Alarm);



	/* ==== Load & Save ==== */

	// Array of all the alarms that haven't been triggered.
	TArray<FTimeClockAlarmData> ActiveAlarms;

	// Adds and saves a new alarm.
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock Alarm"), Category = "Time Clock Alarm")
void AddAlarm(FTimeClockAlarmData Alarm);

	// Removes and saves an alarm if it exists.
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock Alarm"), Category = "Time Clock Alarm")
void RemoveAlarm(FTimeClockAlarmData Alarm);

	// Returns all saved alarms.
	UFUNCTION(BlueprintCallable, meta = (Keywords = "Time Clock Alarm"), Category = "Time Clock Alarm")
	TArray<FTimeClockAlarmData> GetAllAlarms();

};
