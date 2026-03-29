// Copyright Lambda Works, Samuel Metters 2019. All rights reserved.
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "TimeClockCore.h"
#include "TimeClockAlarm.h"

#define TIMECLOCK_MODULE_NAME TEXT("TimeClock")
DECLARE_LOG_CATEGORY_EXTERN(TimeClock, Log, All)

class FTimeClockModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

	// Registers the plugin's settings as project settings.
	void RegisterSettings();

	

	/* ==== Window dropdown button ==== */
	
		// Maps of commands and thier bindings
	TSharedPtr<FUICommandList> PluginCommands;

	// Binds commands and map them.
	void RegisterCommands();

	// Registers TimeClock commands to the "Window" dropdown.
	void RegisterWindowButton();

	// Spawns the command buttons. Happens when the "Window" dropdown is clicked.
	void CreateWindowButton(FMenuBuilder& MenuBuilder);

	// Opens the main interface.
	void OpenTimeClock();



public:

	/* ==== TimeClock Objects ==== */

	UTimeClockCore* TimeClockCoreObject;
	UTimeClockAlarm* TimeClockAlarmObject;

	void GetTimeClockCoreObject(UTimeClockCore* &OutTimeClockObject);
	void GetTimeClockAlarmObject(UTimeClockAlarm* &OutTimeClockAlarmObject);

	// Creates and releases TimeClock objects (such as Core and Alarm).
	void InitialiseTimeClockObjects();
	void ReleaseTimeClockObjects();


};
