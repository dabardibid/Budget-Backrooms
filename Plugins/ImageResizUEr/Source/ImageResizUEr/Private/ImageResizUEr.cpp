// ImageResizUEr
// Copyright 2015-2022 Turfster / NT Entertainment
// All Rights Reserved.


#include "ImageResizUEr.h"

//#include "SlateBasics.h"
//#include "SlateExtras.h"

#include "ImageResizUErStyle.h"
#include "ImageResizUErCommands.h"

#include "LevelEditor.h"

#include "SImageResizeWidget.h"

static const FName ImageResizUErTabName("ImageResizUEr");

#define LOCTEXT_NAMESPACE "FImageResizUErModule"

void FImageResizUErModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FImageResizUErStyle::Initialize();
	FImageResizUErStyle::ReloadTextures();

	FImageResizUErCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FImageResizUErCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FImageResizUErModule::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FImageResizUErModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FImageResizUErModule::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ImageResizUErTabName, FOnSpawnTab::CreateRaw(this, &FImageResizUErModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FImageResizUErTabTitle", "ResizUEr"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FImageResizUErModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FImageResizUErStyle::Shutdown();

	FImageResizUErCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ImageResizUErTabName);
}

TSharedRef<SDockTab> FImageResizUErModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			[
				SNew(SImageResizeWidget)
			]
		];
}

void FImageResizUErModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ImageResizUErTabName);
}

void FImageResizUErModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FImageResizUErCommands::Get().OpenPluginWindow);
}

void FImageResizUErModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FImageResizUErCommands::Get().OpenPluginWindow);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FImageResizUErModule, ImageResizUEr)