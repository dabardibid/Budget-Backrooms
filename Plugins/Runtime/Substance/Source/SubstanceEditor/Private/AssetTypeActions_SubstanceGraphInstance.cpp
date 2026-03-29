// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: AssetTypeActions_SubstanceGraphInstance.h

#include "AssetTypeActions_SubstanceGraphInstance.h"
#include "SubstanceEditorPrivatePCH.h"

#include "SubstanceCoreClasses.h"
#include "SubstanceCoreHelpers.h"
#include "SubstanceGraphInstance.h"
#include "SubstanceInstanceFactory.h"
#include "SubstanceEditorModule.h"

#include "substance/framework/preset.h"
#include "substance/framework/graph.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistryModule.h"
#include "ReferencedAssetsUtils.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

void FAssetTypeActions_SubstanceGraphInstance::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	//Create an array of parent factories that can be used for reimporting
	TArray<UObject*> RawParentFactories;
	TArray<UObject*>::TConstIterator GraphItr = InObjects.CreateConstIterator();
	for (; GraphItr; ++GraphItr)
	{
		USubstanceGraphInstance* CurrentGraph = Cast<USubstanceGraphInstance>(*GraphItr);
		if (CurrentGraph && CurrentGraph->ParentFactory)
		{
			RawParentFactories.Add(Cast<UObject>(CurrentGraph->ParentFactory));
		}

		auto Graphs = GetTypedWeakObjectPtrs<USubstanceGraphInstance>(InObjects);

		MenuBuilder.AddMenuEntry(
		    LOCTEXT("SubstanceGraphInstance_Edit", "Edit"),
		    LOCTEXT("SubstanceGraphInstance_EditTooltip", "Opens the selected graph instances in the graph instance editor."),
		    FSlateIcon(),
		    FUIAction(
		        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceGraphInstance::ExecuteEdit, Graphs),
		        FCanExecuteAction()
		    )
		);

		//Only add the reimport button when there is a valid parent factory to reimport from.
		if (RawParentFactories.Num())
		{
			auto ParentFactories = GetTypedWeakObjectPtrs<USubstanceInstanceFactory>(RawParentFactories);

			MenuBuilder.AddMenuEntry(
			    LOCTEXT("SubstanceGraphInstance_Reimport", "Reimport"),
			    LOCTEXT("SubstanceGraphInstance_ReimportTooltip", "Reimports the selected graph instances."),
			    FSlateIcon(),
			    FUIAction(
			        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceGraphInstance::ExecuteReimport, ParentFactories),
			        FCanExecuteAction()
			    )
			);
		}

		MenuBuilder.AddSubMenu(
		    LOCTEXT("SubstanceGraphInstance_Preset", "Presets"),
		    LOCTEXT("SubstanceGraphInstance_PresetsTooltip", "Deal with presets."),
		    FNewMenuDelegate::CreateRaw(this, &FAssetTypeActions_SubstanceGraphInstance::CreatePresetsSubMenu, Graphs)
		);

		MenuBuilder.AddMenuEntry(
		    LOCTEXT("SubstanceGraphInstance_FindSubstanceInstanceFactory", "Find Instance Factory"),
		    LOCTEXT("SubstanceGraphInstance_FindSubstanceInstanceFactory", "Find parent instance Factory in content browser."),
		    FSlateIcon(),
		    FUIAction(
		        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceGraphInstance::ExecuteFindParentFactory, Graphs),
		        FCanExecuteAction()
		    )
		);

		MenuBuilder.AddMenuEntry(
		    LOCTEXT("SubstanceGraphInstance_DeleteWithOutputs", "Delete with outputs"),
		    LOCTEXT("SubstanceGraphInstance_DeleteWithOutputs", "Delete this instance with all its output textures."),
		    FSlateIcon(),
		    FUIAction(
		        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceGraphInstance::ExecuteDeleteWithOutputs, Graphs),
		        FCanExecuteAction()
		    )
		);
	}
}

void FAssetTypeActions_SubstanceGraphInstance::CreatePresetsSubMenu(class FMenuBuilder& MenuBuilder, TArray<TWeakObjectPtr<USubstanceGraphInstance>> Graphs)
{
	MenuBuilder.AddMenuEntry(
	    LOCTEXT("SubstanceGraphInstance_Import_Presets", "Import Presets"),
	    LOCTEXT("SubstanceGraphInstance_Import_presetsTooltip", "Import presets"),
	    FSlateIcon(),
	    FUIAction(
	        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceGraphInstance::ExecuteImportPresets, Graphs),
	        FCanExecuteAction()
	    )
	);
	MenuBuilder.AddMenuEntry(
	    LOCTEXT("SubstanceGraphInstance_Export_Presets", "Export Presets"),
	    LOCTEXT("SubstanceGraphInstance_Export_presetsTooltip", "Export presets"),
	    FSlateIcon(),
	    FUIAction(
	        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceGraphInstance::ExecuteExportPresets, Graphs),
	        FCanExecuteAction()
	    )
	);

	MenuBuilder.AddMenuEntry(
	    LOCTEXT("SubstanceGraphInstance_ResetInstance", "Reset default values"),
	    LOCTEXT("SubstanceGraphInstance_ResetInstanceTooltip", "Reset Inputs to default values"),
	    FSlateIcon(),
	    FUIAction(
	        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceGraphInstance::ExecuteResetDefault, Graphs),
	        FCanExecuteAction()
	    )
	);
}

void FAssetTypeActions_SubstanceGraphInstance::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Graph = Cast<USubstanceGraphInstance>(*ObjIt);
		if (Graph && Graph->Instance && Graph->ParentFactory)
		{
			ISubstanceEditorModule* SubstanceEditorModule = &FModuleManager::LoadModuleChecked<ISubstanceEditorModule>("SubstanceEditor");
			SubstanceEditorModule->CreateSubstanceEditor(EditWithinLevelEditor, Graph);
		}
		else
		{
			FText NotificationText;
			FFormatNamedArguments Args;
			Args.Add(TEXT("ObjectName"), FText::FromString(Graph->GetName()));
			NotificationText = FText::Format(FText::FromString(TEXT("{ObjectName} is missing Instance Factory, edit disabled. Please delete object.")), Args);

			FNotificationInfo Info(NotificationText);
			Info.ExpireDuration = 5.0f;
			Info.bUseLargeFont = false;
			TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
			if (Notification.IsValid())
			{
				Notification->SetCompletionState(SNotificationItem::CS_Fail);
			}
		}
	}
}

void FAssetTypeActions_SubstanceGraphInstance::ExecuteEdit(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects)
{
	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Object = (*ObjIt).Get();
		if (Object)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Object);
		}
	}
}

void FAssetTypeActions_SubstanceGraphInstance::ExecuteReimport(TArray<TWeakObjectPtr<USubstanceInstanceFactory>> Objects)
{
	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Object = (*ObjIt).Get();
		if (Object)
		{
			FReimportManager::Instance()->Reimport(Object, true, true);
		}
	}
}

void FAssetTypeActions_SubstanceGraphInstance::ExecuteImportPresets(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects)
{
	TArray<TWeakObjectPtr<USubstanceGraphInstance>>::TIterator ObjIt(Objects);
	for (; ObjIt; ++ObjIt)
	{
		Substance::Helpers::ImportAndApplyPresetForGraph((*ObjIt).Get());
	}
	return;
}

void FAssetTypeActions_SubstanceGraphInstance::ExecuteExportPresets(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects)
{
	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		auto Object = (*ObjIt).Get();
		if (Object)
		{
			Substance::Helpers::ExportPresetFromGraph((USubstanceGraphInstance*)Object);
		}
	}
}

void FAssetTypeActions_SubstanceGraphInstance::ExecuteResetDefault(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects)
{
	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		USubstanceGraphInstance* Graph = (*ObjIt).Get();
		if (Graph)
		{
			Substance::Helpers::ResetToDefault(Graph->Instance);
			Graph->PrepareOutputsForSave();
			Substance::Helpers::RenderSync(Graph->Instance, true);
			Graph->SaveAllOutputs();
		}
	}
}

void FAssetTypeActions_SubstanceGraphInstance::ExecuteFindParentFactory(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects)
{
	// select a single object
	if (Objects.Num() == 1)
	{
		USubstanceGraphInstance* Graph = (USubstanceGraphInstance*)(Objects[0].Get());
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		TArray<UObject*> ParentContainer;
		ParentContainer.AddUnique(Graph->ParentFactory);
		ContentBrowserModule.Get().SyncBrowserToAssets(ParentContainer);
	}
}

void FAssetTypeActions_SubstanceGraphInstance::ExecuteDeleteWithOutputs(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects)
{
	for (auto ObjIt = Objects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		USubstanceGraphInstance* Graph = (USubstanceGraphInstance*)((*ObjIt).Get());
		if (Graph)
		{
			// destroy selected graph instance
			Substance::Helpers::RegisterForDeletion(Graph);
		}
	}
}

#undef LOCTEXT_NAMESPACE
