// Corentin GUILLAUME 2020  All Rights Reserved.


#include "OpenNewWindow.h"
#include "LDAssistant.h"
#include "LDAssistantStyle.h"
#include "LDAssistantCommands.h"
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

//AActor* UOpenNewWindow::OpenWindow(AActor* InputActor)
AActor* UOpenNewWindow::OpenWindow() // Note: put a U before the name of your library
{
	UObject* WidgetObj = LoadObject<UObject>(nullptr, TEXT("/LDAssistant/Utilities.Utilities"));
	UEditorUtilityWidgetBlueprint* WidgetBP = (UEditorUtilityWidgetBlueprint*)WidgetObj;
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	EditorUtilitySubsystem->SpawnAndRegisterTab(WidgetBP);
	return 0;
}