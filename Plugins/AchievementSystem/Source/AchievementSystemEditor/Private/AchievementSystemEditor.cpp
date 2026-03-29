// Copyright 2021 Sam Carey. All Rights Reserved.

#include "AchievementSystemEditor.h"
#include "AchievementPinFactory.h"

DEFINE_LOG_CATEGORY(AchievementSystemEditor);

#define LOCTEXT_NAMESPACE "FAchievementSystemEditor"

void FAchievementSystemEditor::StartupModule()
{
	//Register achievement combo Box pin Factory
	BlueprintGraphPanelPinFactory = MakeShareable(new FAchievementPinFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(BlueprintGraphPanelPinFactory);
	
	UE_LOG(AchievementSystemEditor, Warning, TEXT("AchievementSystemEditor module has been loaded"));
}

void FAchievementSystemEditor::ShutdownModule()
{
	//Unregister achievement combo Box pin Factory
	FEdGraphUtilities::UnregisterVisualPinFactory(BlueprintGraphPanelPinFactory);
	
	UE_LOG(AchievementSystemEditor, Warning, TEXT("AchievementSystemEditor module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAchievementSystemEditor, AchievementSystemEditor)