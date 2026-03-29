// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "LDAssistant.h"
#include "LDAssistantStyle.h"
#include "LDAssistantCommands.h"
#include "OpenNewWindow.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Blutility/Public/EditorUtilitySubsystem.h"
#include "Editor/Blutility/Public/IBlutilityModule.h"
#include "Editor/Blutility/Classes/EditorUtilityWidgetBlueprint.h"
#include "UMGEditor/Public/WidgetBlueprint.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectBaseUtility.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

static const FName LDAssistantTabName("LDAssistant");

#define LOCTEXT_NAMESPACE "FLDAssistantModule"

void FLDAssistantModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLDAssistantStyle::Initialize();
	FLDAssistantStyle::ReloadTextures();

	FLDAssistantCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FLDAssistantCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FLDAssistantModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowGlobalTabSpawners", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FLDAssistantModule::AddMenuExtension));
		//MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FLDAssistantModule::AddMenuExtension));
		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FLDAssistantModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
}

void FLDAssistantModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FLDAssistantStyle::Shutdown();

	FLDAssistantCommands::Unregister();
}

void FLDAssistantModule::PluginButtonClicked()
{
	/*Open the basic window*/
	UObject* WidgetObj = LoadObject<UObject>(nullptr, TEXT("/LDAssistant/LDAssistant.LDAssistant"));
	UEditorUtilityWidgetBlueprint* WidgetBP = (UEditorUtilityWidgetBlueprint*)WidgetObj;
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	EditorUtilitySubsystem->SpawnAndRegisterTab(WidgetBP);
}

void FLDAssistantModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FLDAssistantCommands::Get().PluginAction);

}

void FLDAssistantModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FLDAssistantCommands::Get().PluginAction);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLDAssistantModule, LDAssistant)