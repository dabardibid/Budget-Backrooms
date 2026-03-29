// Copyright Lambda Works, Samuel Metters 2019. All rights reserved.
// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "TimeClock.h"
#include "TimeClockCommands.h"
#include "TimeClockCore.h"
#include "TimeClockAlarm.h"
#include "TimeClockSettings.h"

#include "ISettingsModule.h"

#include "LevelEditor.h"
#include "Editor/Blutility/Classes/EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"



#define LOCTEXT_NAMESPACE "FTimeClockModule"
DEFINE_LOG_CATEGORY(TimeClock);


void FTimeClockModule::StartupModule()
{
	UE_LOG(TimeClock, Display, TEXT("TimeClock module startup."));

	RegisterSettings();

	RegisterCommands();

	RegisterWindowButton();

	InitialiseTimeClockObjects();
}

void FTimeClockModule::ShutdownModule()
{
	FTimeClockCommands::Unregister();
	ReleaseTimeClockObjects();
}

void FTimeClockModule::RegisterCommands()
{
	FTimeClockCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FTimeClockCommands::Get().OpenTimeClockCommand,
		FExecuteAction::CreateRaw(this, &FTimeClockModule::OpenTimeClock)
	);
}

void FTimeClockModule::RegisterWindowButton()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	{
		TSharedPtr<FExtender> NewMenuExtender = MakeShareable(new FExtender);
		NewMenuExtender->AddMenuExtension("LevelEditor",
			EExtensionHook::After,
			PluginCommands,
			FMenuExtensionDelegate::CreateRaw(this, &FTimeClockModule::CreateWindowButton));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(NewMenuExtender);

		UE_LOG(TimeClock, Display, TEXT("TimeClock window button added."));
	}
}

void FTimeClockModule::CreateWindowButton(FMenuBuilder& MenuBuilder)
{

	MenuBuilder.BeginSection("CustomMenu", TAttribute<FText>(FText::FromString("Time Clock Plugin")));

	MenuBuilder.AddMenuEntry(FTimeClockCommands::Get().OpenTimeClockCommand);

	MenuBuilder.EndSection();
}

void FTimeClockModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Time Clock",
			LOCTEXT("RuntimeSettingsName", "Time Clock"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the Time Clock settings"),
			GetMutableDefault<UTimeClockSettings>());
	}
}

void FTimeClockModule::OpenTimeClock()
{
	UE_LOG(TimeClock, Display, TEXT("TimeClock widget opening."));

	FString PathToWidget = FString("EditorUtilityWidgetBlueprint'/TimeClock/TimeClock.TimeClock'");

	UObject* WidgetReference = LoadObject<UObject>(nullptr, *PathToWidget);
	UEditorUtilityWidgetBlueprint* Widget = (UEditorUtilityWidgetBlueprint*)WidgetReference;
	if (!WidgetReference) {
		UE_LOG(TimeClock, Error, TEXT("Could not find the TimeClock widget. Make sure there is a EditorUtilityWidget called 'TimeClock' in TimeClock/"));
		return;
	}
	if (!WidgetReference) {
		UE_LOG(TimeClock, Error, TEXT("Failed to cast to the TimeClock EditorUtilityWidget."));
		return;
	}

	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	EditorUtilitySubsystem->SpawnAndRegisterTab(Widget);
}


void FTimeClockModule::InitialiseTimeClockObjects()
{
	TimeClockCoreObject = NewObject<UTimeClockCore>();
	TimeClockCoreObject->InitialiseTimeClockCore();

	TimeClockAlarmObject = NewObject<UTimeClockAlarm>();
	TimeClockAlarmObject->InitialiseTimeClockAlarm();
}

void FTimeClockModule::ReleaseTimeClockObjects()
{
	if (TimeClockCoreObject)
	{
		//TimeClockCoreObject->ConditionalBeginDestroy();
	}

	if (TimeClockAlarmObject)
	{

	}
}

void FTimeClockModule::GetTimeClockCoreObject(UTimeClockCore* &OutTimeClockObject)
{
	OutTimeClockObject = TimeClockCoreObject;
	return;

}


void FTimeClockModule::GetTimeClockAlarmObject(UTimeClockAlarm* &OutTimeClockAlarmObject)
{
	OutTimeClockAlarmObject = TimeClockAlarmObject;
	return;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTimeClockModule, TimeClock)