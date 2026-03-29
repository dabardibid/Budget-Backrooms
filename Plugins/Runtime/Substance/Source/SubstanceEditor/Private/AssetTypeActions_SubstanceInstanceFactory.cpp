// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: AssetTypeActions_SubstanceInstanceFactory.cpp

#include "AssetTypeActions_SubstanceInstanceFactory.h"
#include "SubstanceEditorPrivatePCH.h"
#include "SubstanceCoreHelpers.h"
#include "SubstanceCoreClasses.h"
#include "SubstanceInstanceFactory.h"
#include "SubstanceFactory.h"

#include "substance/framework/graph.h"
#include "substance/framework/package.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetTypeActions_Base.h"
#include "AssetRegistryModule.h"
#include "ReferencedAssetsUtils.h"
#include "Internationalization/Text.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

void FAssetTypeActions_SubstanceInstanceFactory::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto InstanceFactory = GetTypedWeakObjectPtrs<USubstanceInstanceFactory>(InObjects);

	MenuBuilder.AddMenuEntry(
	    LOCTEXT("SubstanceInstanceFactory_Reimport", "Reimport"),
	    LOCTEXT("SubstanceInstanceFactory_ReimportTooltip", "Reimports the selected graph instances."),
	    FSlateIcon(),
	    FUIAction(
	        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceInstanceFactory::ExecuteReimport, InstanceFactory),
	        FCanExecuteAction()
	    )
	);

	//Show list of graph contained in instance factory when a single one is selected
	if (InObjects.Num() == 1)
	{
		MenuBuilder.AddSubMenu(
		    LOCTEXT("SubstanceInstanceFactory_CreateInstance", "Create Graph Instance"),
		    LOCTEXT("SubstanceInstanceFactory_CreateInstance", "Create an instance of the specified graph."),
		    FNewMenuDelegate::CreateRaw(this, &FAssetTypeActions_SubstanceInstanceFactory::CreateInstanceSubMenu, InstanceFactory)
		);
	}

	MenuBuilder.AddMenuEntry(
	    LOCTEXT("SubstanceInstanceFactory_DeleteWithOutputs", "Delete with instances and outputs"),
	    LOCTEXT("SubstanceInstanceFactory_DeleteWithOutputs", "Delete this instanceFactory with all its instances and output textures."),
	    FSlateIcon(),
	    FUIAction(
	        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceInstanceFactory::ExecuteDeleteWithOutputs, InstanceFactory),
	        FCanExecuteAction()
	    )
	);
}

void FAssetTypeActions_SubstanceInstanceFactory::CreateInstanceSubMenu(class FMenuBuilder& MenuBuilder, TArray<TWeakObjectPtr<USubstanceInstanceFactory>> InstanceFactory)
{
	//Show list of graph contained in instance factory when a single one is selected
	for (const auto& ItGraphDesc : InstanceFactory[0]->SubstancePackage->getGraphs())
	{
		FString CurrentGraphLabel(ItGraphDesc.mLabel.c_str());
		FText text = FText::FromString(FString::Printf(TEXT("New Instance of %s"), *CurrentGraphLabel));

		FString CurrentName = InstanceFactory[0]->GetName();
		MenuBuilder.AddMenuEntry(
		    text,
		    FText::FromString(TEXT("")),
		    FSlateIcon(),
		    FUIAction(
		        FExecuteAction::CreateSP(this, &FAssetTypeActions_SubstanceInstanceFactory::ExecuteInstantiate, &ItGraphDesc, InstanceFactory[0].Get()),
		        FCanExecuteAction()
		    ));
	}
}

void FAssetTypeActions_SubstanceInstanceFactory::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
}

void FAssetTypeActions_SubstanceInstanceFactory::ExecuteReimport(TArray<TWeakObjectPtr<USubstanceInstanceFactory>> Objects)
{
	for (const auto& ObjIt : Objects)
	{
		if (ObjIt.IsValid())
		{
			FReimportManager::Instance()->Reimport(ObjIt.Get(), true, true);
		}
	}
}

void FAssetTypeActions_SubstanceInstanceFactory::ExecuteInstantiate(const SubstanceAir::GraphDesc* Desc, USubstanceInstanceFactory* Factory)
{
	bool bOperationCanceled = false;
	Substance::FSubstanceImportOptions ImportOptions;
	ImportOptions.bForceCreateInstance = true;

	Substance::GetImportOptions(FString(Desc->mLabel.c_str()), Factory->GetOuter()->GetName(), ImportOptions, bOperationCanceled);

	if (bOperationCanceled)
	{
		return;
	}

	UObject* InstanceOuter = CreatePackage(*ImportOptions.InstanceDestinationPath);
	USubstanceGraphInstance* Instance = Substance::Helpers::InstantiateGraph(Factory, *Desc, InstanceOuter, ImportOptions.InstanceName, true, RF_Standalone | RF_Public, ImportOptions.ParentMaterial);
	Instance->PrepareOutputsForSave();

	TArray<int32> OutputSize;
	OutputSize.Add(GetDefault<USubstanceSettings>()->DefaultSubstanceOutputSizeX);
	OutputSize.Add(GetDefault<USubstanceSettings>()->DefaultSubstanceOutputSizeY);

	Instance->SetInputInt(TEXT("$outputsize"), OutputSize);

	Substance::Helpers::RenderSync(Instance->Instance, true);
	Instance->SaveAllOutputs();

	if (ImportOptions.bCreateMaterial)
	{
		UObject* MaterialParent = CreatePackage(*ImportOptions.MaterialDestinationPath);
		Substance::Helpers::CreateMaterial(Instance, ImportOptions.MaterialName, MaterialParent);
	}

	TArray<UObject*> AssetList;
	AssetList.AddUnique(Instance);
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(AssetList, true);
}

void FAssetTypeActions_SubstanceInstanceFactory::ExecuteDeleteWithOutputs(TArray<TWeakObjectPtr<USubstanceInstanceFactory>> Objects)
{
	for (const auto& ObjIt : Objects)
	{
		USubstanceInstanceFactory* InstanceFactory = (USubstanceInstanceFactory*)(ObjIt.Get());
		if (InstanceFactory)
		{
			InstanceFactory->ClearReferencingObjects();
			Substance::Helpers::RegisterForDeletion(InstanceFactory);
		}
	}
}

#undef LOCTEXT_NAMESPACE
