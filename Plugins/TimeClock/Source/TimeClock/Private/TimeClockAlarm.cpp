// Copyright Lambda Works, Samuel Metters 2019. All rights reserved.


#include "TimeClockAlarm.h"
#include "LevelEditor.h"
#include "TimeClockBPLibrary.h"
#include "TimeClockSettings.h"
#include "TimeClockNotificationManager.h"

UTimeClockAlarm::UTimeClockAlarm()
{

}

UTimeClockAlarm::~UTimeClockAlarm()
{
	UObjectBaseUtility::RemoveFromRoot();
}

void UTimeClockAlarm::InitialiseTimeClockAlarm()
{
	// Necessary to avoid random GC
	UObjectBaseUtility::AddToRoot();

	// Load all alarms
	ActiveAlarms = GetAllAlarms();
	
	// Make sure the alarm timer only starts once the editor has booted.
	StartupDelegate = GEditor->OnPostEditorTick().AddUFunction(this, FName("StartAlarmTimer"));
}

void UTimeClockAlarm::StartAlarmTimer()
{
	UE_LOG(TimeClock, Warning, TEXT("TimeClock Alarm started"));

	if (!AlarmTimerHandle.IsValid())
	{
		// Start the timer
		GEditor->GetTimerManager().Get().SetTimer(AlarmTimerHandle, this, &UTimeClockAlarm::TimeClockAlarmTick, AlarmTimerRate, true, 4.0f);

		// Unbind the function from EditorTick
		GEditor->OnPostEditorTick().Remove(StartupDelegate);
	}
}

void UTimeClockAlarm::TimeClockAlarmTick()
{
	TArray<FTimeClockAlarmData> AlarmsToTrigger;
	FDateTime CurrentTime = FDateTime::Now();

	// For each registered alarms
	for (int i = 0; i < ActiveAlarms.Num(); i++)
	{
		// Is the hour value the same
		if (ActiveAlarms[i].AlarmTime.GetHour() == CurrentTime.GetHour())
		{
			// Is the minute value +/- 5 minutes the same.
			if (ActiveAlarms[i].AlarmTime.GetMinute() == CurrentTime.GetMinute())
			{
				// Add it to temp array
				AlarmsToTrigger.Add(ActiveAlarms[i]);
			}
		}
	}

	// For each alarm that have to be triggered
	if (AlarmsToTrigger.Num() > 0)
	{
		// Reverse loop as we'll be removing them to ensure they're not triggered again.
		for (int j = AlarmsToTrigger.Num() - 1; j >= 0; j--)
		{
			TriggerAlarm(AlarmsToTrigger[j]);
			ActiveAlarms.Remove(AlarmsToTrigger[j]);
		}
	}
}

void UTimeClockAlarm::TriggerAlarm(FTimeClockAlarmData Alarm)
{
	// Send editor notification
	FTimeClockNotificationManager::Get()->DispatchNotification(Alarm.AlarmText, AlarmDuration, true);

	UE_LOG(TimeClock, Warning, TEXT("TimeClock Alarm: %s"), *Alarm.AlarmText);
}

void UTimeClockAlarm::AddAlarm(FTimeClockAlarmData Alarm)
{
	// Add the alarm to the active pool of alarms.
	ActiveAlarms.AddUnique(Alarm);	
	
	// Add the alarm to the config file.
	UTimeClockSettings* TimeClockSettings = GetMutableDefault<UTimeClockSettings>();
	TimeClockSettings->Alarms.AddUnique(Alarm);
	TimeClockSettings->UpdateDefaultConfigFile();
}

void UTimeClockAlarm::RemoveAlarm(FTimeClockAlarmData Alarm)
{
	// Remove the alarm from the active pool if present
	if (ActiveAlarms.Contains(Alarm))
	{
		ActiveAlarms.Remove(Alarm);
	}

	// Remove the alarm from the config file.
	UTimeClockSettings* TimeClockSettings = GetMutableDefault<UTimeClockSettings>();

	if (TimeClockSettings->Alarms.Contains(Alarm))
	{
		TimeClockSettings->Alarms.RemoveSingle(Alarm);
		TimeClockSettings->UpdateDefaultConfigFile();
	}
}

TArray<FTimeClockAlarmData> UTimeClockAlarm::GetAllAlarms()
{
	UTimeClockSettings* TimeClockSettings = GetMutableDefault<UTimeClockSettings>();
	return TimeClockSettings->Alarms;
}
