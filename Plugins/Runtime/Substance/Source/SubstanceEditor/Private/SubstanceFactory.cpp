// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceFactory.cpp

#include "SubstanceFactory.h"
#include "SubstanceEditorPrivatePCH.h"

#include "SubstanceEditorClasses.h"
#include "SubstanceOptionWindow.h"
#include "SubstanceCoreHelpers.h"
#include "SubstanceCoreClasses.h"

#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AutomatedAssetImportData.h"
#include "ObjectTools.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Interfaces/IMainFrameModule.h"
#include "Misc/Paths.h"

#include "Materials/MaterialInstance.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "FileHelpers.h"

#include "substance/framework/output.h"
#include "substance/framework/input.h"
#include "substance/framework/graph.h"
#include "substance/framework/package.h"
#include "substance/framework/preset.h"
#include "substance/framework/typedefs.h"

//==============================================================================================================
// ReimportData is a struct created per graph instance that needs to be recreated post reimport of a factory
struct ReimportData
{
	//#TODO:: Save the USubstanceGraphInstance package name along with the USubstanceTexture2D package name
	//in the case the user renamed any of the packages through the content browser. Restore these names over recreating new default unique names.

	//UAsset package name needed to prevent rename loss
	FString UAssetName;

	//PackageURL of the graph used to create this instance
	FString PackageURL;

	//Reference to the parent material that was used to create the material instance
	UMaterial* ParentMaterial;

	//Reference to the material that was created with this instance (if created)
	UMaterialInstanceConstant* OwnedMaterialReferencee;

	//Map of all of the enabled output UIDs and their corresponding UAsset package names
	TMap<uint32, OutputTextureSettings> EnabledOutputUIDs;

	//All of the referenced image inputs assigned for this graph
	TMap<uint32, UTexture2D*> ImageSources;

	//List of all of the materials that reference an output created from this graph
	TArray<MaterialParameterSet> ReferencingMaterials;

	//List of all of the material dynamic instances that reference an output created from this graph
	TArray<MaterialInstanceParameterSet> ReferencingMaterialInstances;

	//Set of all of the previous input changes that were applied before the reimport
	SubstanceAir::shared_ptr<SubstanceAir::Preset> PreviousPresetData;

	ReimportData(const FString& packageURL
	             , FString assetName
				 , UMaterial* parentMaterial
	             , UMaterialInstanceConstant* owningMaterial
	             , SubstanceAir::shared_ptr<SubstanceAir::Preset> preset
	             , TArray<MaterialParameterSet>& referencingMaterials
	             , TArray<MaterialInstanceParameterSet>& referencingMaterialInstances
	             , TMap<uint32, UTexture2D*>& imageSources
	             , TMap<uint32, OutputTextureSettings>& enabledOutputUIDs)
	{
		UAssetName = assetName;
		PackageURL = packageURL;
		ImageSources = imageSources;
		PreviousPresetData = preset;
		EnabledOutputUIDs = enabledOutputUIDs;
		ReferencingMaterials = referencingMaterials;
		ParentMaterial = parentMaterial;
		OwnedMaterialReferencee = owningMaterial;
		ReferencingMaterialInstances = referencingMaterialInstances;
	}
};

/** Reimport data map */
TArray<ReimportData> PreviousInstanceTransfer;

namespace local
{
bool bIsPerformingReimport = false;
USubstanceInstanceFactory* FactoryBeingReimported;
}

void Substance::ApplyImportUIToImportOptions(USubstanceImportOptionsUi* ImportUI, FSubstanceImportOptions& InOutImportOptions)
{
	static FName AssetToolsModuleName = FName("AssetTools");
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(AssetToolsModuleName);
	FString PkgName;
	FString AssetName;

	InOutImportOptions.bCreateInstance = ImportUI->bCreateInstance;
	InOutImportOptions.bCreateMaterial = ImportUI->bCreateMaterial;

	InOutImportOptions.ParentType = ImportUI->uMaterialParentType;
	InOutImportOptions.ParentMaterial = ImportUI->ParentMaterial;

	AssetToolsModule.Get().CreateUniqueAssetName(ImportUI->InstanceDestinationPath + TEXT("/"), ImportUI->InstanceName, PkgName, AssetName);

	InOutImportOptions.InstanceDestinationPath = PkgName;
	InOutImportOptions.InstanceName = AssetName;

	AssetToolsModule.Get().CreateUniqueAssetName(ImportUI->MaterialDestinationPath + TEXT("/"), ImportUI->MaterialName, PkgName, AssetName);

	InOutImportOptions.MaterialName = AssetName;
	InOutImportOptions.MaterialDestinationPath = PkgName;
}

USubstanceFactory::USubstanceFactory(const class FObjectInitializer& PCIP)
	: Super(PCIP)
{
	bEditAfterNew = true;
	bEditorImport = true;

	SupportedClass = USubstanceInstanceFactory::StaticClass();

	Formats.Empty(1);
	Formats.Add(TEXT("sbsar;Substance Texture"));
}

void Substance::GetImportOptions(
    FString Name,
    FString ParentName,
    FSubstanceImportOptions& InOutImportOptions,
    bool& OutOperationCanceled)
{
	USubstanceImportOptionsUi* ImportUI = NewObject<USubstanceImportOptionsUi>();

	ImportUI->bForceCreateInstance = InOutImportOptions.bForceCreateInstance;
	ImportUI->bCreateInstance = true;
	ImportUI->bCreateMaterial = true;

	Name = ObjectTools::SanitizeObjectName(Name);

	ImportUI->bOverrideFullName = false;
	FString BasePath;
	ParentName.Split(TEXT("/"), &(BasePath), nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	ImportUI->InstanceDestinationPath = BasePath;
	ImportUI->MaterialDestinationPath = BasePath;

	FString FullAssetPath;
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
	AssetToolsModule.Get().CreateUniqueAssetName(BasePath + TEXT("/") + Name + TEXT("_INST"), TEXT(""),
	        FullAssetPath, ImportUI->InstanceName);

	AssetToolsModule.Get().CreateUniqueAssetName(BasePath + TEXT("/") + Name + TEXT("_MAT"), TEXT(""),
	        FullAssetPath, ImportUI->MaterialName);

	TSharedPtr<SWindow> ParentWindow;
	//Check if the main frame is loaded.  When using the old main frame it may not be.
	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		ParentWindow = MainFrame.GetParentWindow();
	}

	TSharedRef<SWindow> Window = SNew(SWindow)
	                             .Title(NSLOCTEXT("UnrealEd", "SubstanceImportOpionsTitle", "Substance Import Options"))
	                             .SizingRule(ESizingRule::UserSized);


	int32 width=0, height=0, x=0, y=0;
	
	if (ParentWindow)
	{
		ParentWindow->GetNativeWindow()->GetFullScreenInfo(x, y, width, height);
	}

	width = std::max<int>(450, width/3);
	height = std::max<int>(600, height/2);

	Window->Resize(FVector2D(width, height));

	TSharedPtr<SSubstanceOptionWindow> SubstanceOptionWindow;
	Window->SetContent
	(
	    SAssignNew(SubstanceOptionWindow, SSubstanceOptionWindow)
	    .ImportUI(ImportUI)
	    .WidgetWindow(Window)
	);

	FVector2D CurrPos = Window->GetInitialDesiredPositionInScreen();
	Window->MoveWindowTo(FVector2D(CurrPos.X - width / 2, CurrPos.Y - height / 2));

	//TODO:: We can make this slow as showing progress bar later
	FSlateApplication::Get().AddModalWindow(Window, ParentWindow, false);

	if (SubstanceOptionWindow->ShouldImport())
	{
		//Open dialog. See if it's canceled
		ApplyImportUIToImportOptions(ImportUI, InOutImportOptions);
		OutOperationCanceled = false;
	}
	else
	{
		OutOperationCanceled = true;
	}
}
UObject* USubstanceFactory::FactoryCreateBinary(
    UClass* Class,
    UObject* InParent,
    FName Name,
    EObjectFlags Flags,
    UObject* Context,
    const TCHAR* Type,
    const uint8*& Buffer,
    const uint8* BufferEnd,
    FFeedbackContext* Warn)
{
	USubstanceInstanceFactory* Factory = nullptr;
	FName InstanceFactoryName = Name;
	UObject* Parent = InParent;

	//Overwrite defaults to specify which object to overwrite during a reimport.
	if (local::bIsPerformingReimport)
	{
		InstanceFactoryName = local::FactoryBeingReimported->GetFName();
		Parent = local::FactoryBeingReimported->GetOuter();

		//Reset as this should not be used after this point until the next reimport.
		local::FactoryBeingReimported = nullptr;
	}
	else
	{
		//Make sure the asset doesn't already exist
		for (TObjectIterator<USubstanceInstanceFactory> Itr; Itr; ++Itr)
		{
			if ((*Itr)->GetFName() == InstanceFactoryName)
			{
				//Highlight the existing assets within the content browser
				FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
				TArray<UObject*> AssetList{ (*Itr) };
				ContentBrowserModule.Get().SyncBrowserToAssets(AssetList, true);

				return *Itr;
			}
		}
	}

	//Create a USubstanceFactory UAsset - This will appear in content browser
	Factory = CastChecked<USubstanceInstanceFactory>(CreateOrOverwriteAsset(USubstanceInstanceFactory::StaticClass(), Parent,
	          InstanceFactoryName, RF_Standalone | RF_Public));

	const uint32 BufferLength = BufferEnd - Buffer;

	//Create the framework package
	Factory->Initialize(Substance::Helpers::InstantiatePackage((void*)Buffer, BufferLength));

	//If the operation failed
	if (false == Factory->SubstancePackage->isValid())
	{
		//Mark the package for garbage collect
		Factory->ClearFlags(RF_Standalone);
		return nullptr;
	}

	//Create relative path
	FString RelativePath = GetCurrentFilename();
	FString RootGameDir = FPaths::ProjectDir();

	//Make relative paths to the source file
	FPaths::MakeStandardFilename(RootGameDir);
	FPaths::MakePathRelativeTo(RelativePath, *RootGameDir);

	//Set the source file data
	Factory->AbsoluteSourceFilePath = GetCurrentFilename();
	Factory->RelativeSourceFilePath = RelativePath;
	Factory->SourceFileTimestamp = IFileManager::Get().GetTimeStamp(*Factory->AbsoluteSourceFilePath).ToString();

	Substance::FSubstanceImportOptions ImportOptions;
	ImportOptions.ParentMaterial = nullptr;


	TArray<FString> Names;

	bool bAllCancel = true;

	//Stores references to all of the assets we are about to create to sync with content browser
	TArray<UObject*> AssetList;

	//Reimport Assets compared to plain import
	if (local::bIsPerformingReimport)
	{
		RecreateGraphsPostReimport(Factory, InParent);
		Factory->MarkPackageDirty();
		return Factory;
	}

	//Create all of the graphs
	for (const auto& GraphIt : Factory->SubstancePackage->getGraphs())
	{
		bool bOperationCanceled = false;

		if (bSuppressImportOverwriteDialog)
		{
			ImportOptions.bCreateInstance = bCreateInstance;
			ImportOptions.bCreateMaterial = bCreateMaterial;

			FString BasePath = InParent->GetName();
			InParent->GetName().Split(TEXT("/"), &(BasePath), nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

			ImportOptions.InstanceDestinationPath = BasePath;
			ImportOptions.MaterialDestinationPath = BasePath;

			FString FullAssetPath;
			FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
			AssetToolsModule.Get().CreateUniqueAssetName(BasePath + TEXT("/") + GraphIt.mLabel.c_str() + TEXT("_INST"), TEXT(""),
			        FullAssetPath, ImportOptions.InstanceName);

			AssetToolsModule.Get().CreateUniqueAssetName(BasePath + TEXT("/") + GraphIt.mLabel.c_str() + TEXT("_MAT"), TEXT(""),
			        FullAssetPath, ImportOptions.MaterialName);

			//Finalize name to match that applied by the import UI
			FString PkgName;
			FString AssetName;

			AssetToolsModule.Get().CreateUniqueAssetName(ImportOptions.InstanceDestinationPath + TEXT("/"), ImportOptions.InstanceName, PkgName, AssetName);

			ImportOptions.InstanceDestinationPath = PkgName;
			ImportOptions.InstanceName = AssetName;

			AssetToolsModule.Get().CreateUniqueAssetName(ImportOptions.MaterialDestinationPath + TEXT("/"), ImportOptions.MaterialName, PkgName, AssetName);

			ImportOptions.MaterialName = AssetName;
			ImportOptions.MaterialDestinationPath = PkgName;
		}
		else
		{
			Substance::GetImportOptions(GraphIt.mLabel.c_str(), InParent->GetName(), ImportOptions, bOperationCanceled);
		}

		if (bOperationCanceled)
		{
			bAllCancel = bOperationCanceled && bAllCancel;
			continue;
		}
		else
		{
			bAllCancel = false;
		}

		if (ImportOptions.bCreateInstance)
		{
			FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
			FString InstPath = ImportOptions.InstanceDestinationPath;
			FString InstName = ImportOptions.InstanceName;

			//Create USubstanceGraphInstance
			UObject* GraphBasePackage = CreatePackage(*InstPath);

			USubstanceGraphInstance* NewInstance = Substance::Helpers::InstantiateGraph(Factory, GraphIt, GraphBasePackage, InstName, true,RF_Standalone |RF_Public, ImportOptions.ParentMaterial);
			
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			AssetRegistryModule.Get().AssetCreated(NewInstance);

			NewInstance->PostLoad();
			NewInstance->CreatedMaterial = ImportOptions.ParentMaterial;

			//Set the default output size once on reimport
			TArray<int32> DefaultOutputSize{ GetDefault<USubstanceSettings>()->DefaultSubstanceOutputSizeX, GetDefault<USubstanceSettings>()->DefaultSubstanceOutputSizeY };
			NewInstance->SetInputInt("$outputsize", DefaultOutputSize);

			NewInstance->PrepareOutputsForSave();
			Substance::Helpers::RenderSync(NewInstance->Instance, true);
			NewInstance->SaveAllOutputs();

			if (ImportOptions.bCreateMaterial)
			{
				FString MatPath = ImportOptions.MaterialDestinationPath;
				FString MatName = ImportOptions.MaterialName;

				UObject* MaterialBasePackage = CreatePackage(*MatPath);
				TWeakObjectPtr<UMaterialInstance> Mat = Substance::Helpers::CreateMaterial(NewInstance, MatName, MaterialBasePackage);
				if (Mat.IsValid())
				{
					AssetList.AddUnique(Mat.Get());
				}
				else
				{
					AssetList.AddUnique(NewInstance);
					AssetList.AddUnique(Factory);
				}
			}
			else
			{
				AssetList.AddUnique(NewInstance);
				AssetList.AddUnique(Factory);
			}
		}
	}

	//Reset Suppression
	bSuppressImportOverwriteDialog = false;

	//If we have created SubstanceGraphInstances and we cancel - They are never cleaned and
	//the package is which will cause a crash
	if (bAllCancel)
	{
		Factory->ClearReferencingObjects();
		Factory->ClearFlags(RF_Standalone);
		CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		return nullptr;
		
	}

	//Update the content browser with the new assets
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(AssetList, true);

	Factory->MarkPackageDirty();

	return Factory;
}

bool USubstanceFactory::FactoryCanImport(const FString& Filename)
{
	static const FString extension(TEXT("sbsar"));
	return Filename.EndsWith(extension, ESearchCase::Type::IgnoreCase);
}

USubstanceInstanceFactory* USubstanceFactory::ImportArchive(const FString& destinationPath, const FString& filename)
{
	// Alternatively could read file directly and use FactoryCreateBinary?
	IAssetTools& AssetTools = FAssetToolsModule::GetModule().Get();
	bCreateInstance = false;
	bCreateMaterial = false;
	bSuppressImportOverwriteDialog = true;
	UAutomatedAssetImportData* autoTask = NewObject<UAutomatedAssetImportData>();
	autoTask->Filenames = { filename };
	autoTask->DestinationPath = destinationPath;
	autoTask->bReplaceExisting = true;
	autoTask->bSkipReadOnly = true;
	autoTask->Factory = this;
	auto assets = AssetTools.ImportAssetsAutomated(autoTask);
	if (assets.Num() <= 0)
	{
		return nullptr;
	}
	return Cast<USubstanceInstanceFactory>(assets[0]);
}

/** Disables the import options for the next import */
void USubstanceFactory::SuppressImportOverwriteDialog()
{
	bSuppressImportOverwriteDialog = true;
}

void USubstanceFactory::CreateInstance(bool value)
{
	bCreateInstance = value;
}

void USubstanceFactory::CreateMaterial(bool value)
{
	bCreateMaterial = value;
}

void USubstanceFactory::RecreateGraphsPostReimport(USubstanceInstanceFactory* ParentFactory, UObject* InParent)
{
	//Stores references to all of the assets we are about to create to sync with content browser
	TArray<UObject*> AssetList;

	//Stores the reference to the graph we will be creating
	const SubstanceAir::GraphDesc* GraphDescToCreate = nullptr;

	//Loop through all of the graphs we need to recreate
	for (const auto& GrItr : PreviousInstanceTransfer)
	{
		//Find the graph to use to create this instance
		for (const auto& GDescItr : ParentFactory->SubstancePackage->getGraphs())
		{
			if (FString(GDescItr.mPackageUrl.c_str()) == GrItr.PackageURL)
			{
				GraphDescToCreate = &GDescItr;
				break;
			}
		}

		//If we can't find the graph instance to create, warn the user and continue gracefully
		if (!GraphDescToCreate)
		{
			UE_LOG(LogSubstanceEditor, Warning, TEXT("Failed to create a graph instance after reimport"))
			continue;
		}

		//Get the readable UAsset name from given the path
		FString BaseInstanceName;
		GrItr.UAssetName.Split(TEXT("/"), nullptr, &(BaseInstanceName), ESearchCase::IgnoreCase, ESearchDir::FromEnd);

		//Create USubstanceGraphInstance
		UObject* GraphBasePackage = CreatePackage(*GrItr.UAssetName);
		USubstanceGraphInstance* NewInstance = Substance::Helpers::InstantiateGraph(ParentFactory, *GraphDescToCreate, GraphBasePackage, BaseInstanceName, 
																					false, RF_Standalone | RF_Public, (UMaterial*)GrItr.ParentMaterial);
		AssetList.AddUnique(NewInstance);

		//Enable all outputs that were previously enabled
		for (const auto& OutputItr : NewInstance->Instance->getOutputs())
		{
			if (!OutputItr->mUserData && GrItr.EnabledOutputUIDs.Contains(OutputItr->mDesc.mUid))
			{
				Substance::Helpers::EnableTexture(OutputItr, NewInstance, &GrItr.EnabledOutputUIDs[OutputItr->mDesc.mUid]);
			}
			else if (!GrItr.EnabledOutputUIDs.Contains(OutputItr->mDesc.mUid) && OutputItr->mUserData)
			{
				TWeakObjectPtr<UTexture2D> Texture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData)->GetData());

				if (Texture.IsValid())
					Substance::Helpers::RegisterForDeletion(Texture.Get());

				NewInstance->OutputInstances.Remove(OutputItr->mDesc.mUid);
			}
		}

		//Set the image input data
		NewInstance->ImageSources = GrItr.ImageSources;

		//Make sure to initialize the graph instance (Primarily used for image input setup)
		NewInstance->PostLoad();

		//Apply presets
		Substance::Helpers::ApplyPresetData(NewInstance->Instance, *GrItr.PreviousPresetData.get());

		//Make sure the textures are recomputed post create
		NewInstance->PrepareOutputsForSave();
		Substance::Helpers::RenderSync(NewInstance->Instance, true);
		NewInstance->SaveAllOutputs();

		//Check if material is valid if so, set it up with the new outputs
		Substance::Helpers::ResetMaterialTexturesFromGraph(NewInstance, GrItr.OwnedMaterialReferencee, GrItr.ReferencingMaterials);

		//Reset any references to substance textures in material instances
		Substance::Helpers::ResetMaterialInstanceTexturesFromGraph(NewInstance, GrItr.ReferencingMaterialInstances);
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(AssetList, true);

	//Clear the recreation data as it is no longer needed
	PreviousInstanceTransfer.Empty();
}

//==============================================================================================================
//Begin UReimportSubstanceFactory Implementation
UReimportSubstanceFactory::UReimportSubstanceFactory(class FObjectInitializer const& PCIP) : Super(PCIP)
{

}

bool UReimportSubstanceFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	const USubstanceGraphInstance* GraphInstance = Cast<USubstanceGraphInstance>(Obj);
	const USubstanceInstanceFactory* InstanceFactory = Cast<USubstanceInstanceFactory>(Obj);

	//Make sure one is valid
	if (nullptr == GraphInstance && nullptr == InstanceFactory)
	{
		return false;
	}

	//Make sure we have a valid parent factory
	if (GraphInstance && nullptr == GraphInstance->ParentFactory)
	{
		UE_LOG(LogSubstanceEditor, Warning, TEXT("Cannot reimport: The Substance Graph Instance does not have any parent package."));
		return false;
	}

	if (GraphInstance)
	{
		InstanceFactory = GraphInstance->ParentFactory;
	}

	//Check absolute file path
	IFileManager& FileManager = IFileManager::Get();
	if (FileManager.FileExists(*InstanceFactory->AbsoluteSourceFilePath))
	{
		OutFilenames.Add(InstanceFactory->AbsoluteSourceFilePath);
		return true;
	}

	//Get absolute path from relative
	FString Blank;
	FString DiskFilename = FileManager.GetFilenameOnDisk(*Blank);

	//Combine relative source path with absolute path to directory to get an absolute source file path
	FString BasePath;
	DiskFilename.Split(TEXT("/Binaries"), &(BasePath), nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	FString AbsoluteFromRelative = FPaths::Combine(BasePath, InstanceFactory->RelativeSourceFilePath);

	//Check relative path
	if (FileManager.FileExists(*AbsoluteFromRelative))
	{
		OutFilenames.Add(AbsoluteFromRelative);
		return true;
	}

	//If neither path is found, we can not reimport
	return false;
}

void UReimportSubstanceFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	USubstanceInstanceFactory* Factory = Cast<USubstanceInstanceFactory>(Obj);
	if (Factory && ensure(NewReimportPaths.Num() == 1))
	{
		Factory->AbsoluteSourceFilePath = NewReimportPaths[0];
	}
}

EReimportResult::Type UReimportSubstanceFactory::Reimport(UObject* Obj)
{
	local::bIsPerformingReimport = true;
	local::FactoryBeingReimported = nullptr;

	//Check to see if we are trying to reimport a graph instance and if so, alert the user that a factory reimport must be preformed
	//#NOTE:: We can not default to reimporting a factory here as it will destroy the graph instance that is trying to be reimported which
	//is also references and accessed post reimport.
	USubstanceGraphInstance* GraphInstanceBeingReimported = Cast<USubstanceGraphInstance>(Obj);
	if (GraphInstanceBeingReimported)
	{
		UE_LOG(LogSubstanceEditor, Warning, TEXT("To reimport changes from a Substance file, the SubstanceInstanceFactory must be reimported"))
		return EReimportResult::Succeeded;
	}

	//Find the parent factory - root of the reimport
	USubstanceInstanceFactory* CurrentInstanceFactory = Cast<USubstanceInstanceFactory>(Obj);

	if (!CurrentInstanceFactory)
	{
		UE_LOG(LogSubstanceEditor, Error, TEXT("Unable to reimport: The instance factory is invalid"))
		local::bIsPerformingReimport = false;
		return EReimportResult::Failed;
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	FString PathName = CurrentInstanceFactory->GetPathName();

	//Check if we can reimport which also will grab to path to reimport from
	TArray<FString> SourcePaths;
	if (!CanReimport(CurrentInstanceFactory, SourcePaths))
	{
		UE_LOG(LogSubstanceEditor, Error, TEXT("Unable to reimport: Cannot find asset at relative or absolute path saved when asset was first created."));
		local::bIsPerformingReimport = false;
		return EReimportResult::Failed;
	}

	//Fix up path
	FString RootPath;
	FString CurrentName;
	PathName.Split(TEXT("/"), &RootPath, &CurrentName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	//Before we clears the objects, store all of the registered graph instances and their materials (if valid)
	//to be able to create all the needed graph instances when reimported and to properly assign the new outputs to the
	//old material.
	SaveRecreationData(CurrentInstanceFactory);

	//Import settings
	UAutomatedAssetImportData* ImportData = NewObject<UAutomatedAssetImportData>();

	//Import again - We already know paths are valid from can reimport
	ImportData->bReplaceExisting = true;
	ImportData->Filenames = SourcePaths;
	ImportData->DestinationPath = RootPath;
	ImportData->bSkipReadOnly = true;

	local::FactoryBeingReimported = CurrentInstanceFactory;

	AssetToolsModule.Get().ImportAssetsAutomated(ImportData);

	//Return success
	UE_LOG(LogSubstanceEditor, Log, TEXT("Re-imported successfully"));
	local::bIsPerformingReimport = false;
	return EReimportResult::Succeeded;
}

/** Saves all of the informations we need to recreate this factory */
void UReimportSubstanceFactory::SaveRecreationData(USubstanceInstanceFactory* FactoryToReimport)
{
	//This should already be empty but making sure we don't try to recreate garbage
	PreviousInstanceTransfer.Empty();

	//Save which graph instance and the material for all graph instances the factory has created
	for (const auto& GraphItr : FactoryToReimport->GetGraphInstances())
	{
		if (!GraphItr)
		{
			continue;
		}
		TArray<MaterialParameterSet> ReferencingMaterials;
		//Fill an array with all materials referencing this graph instance and pass it to the reimport data
		for (TObjectIterator<UMaterial> MatItr; MatItr; ++MatItr)
		{
			//The data set for the current material to be filled out if the material references outputs from this graph
			MaterialParameterSet CurrentMaterialSet;

			//Create the material data
			for (int32 ExpressionIndex = (*MatItr)->Expressions.Num() - 1; ExpressionIndex >= 0; ExpressionIndex--)
			{
				UMaterialExpressionTextureSample* Expression = Cast<UMaterialExpressionTextureSample>((*MatItr)->Expressions[ExpressionIndex]);

				if (Expression && Expression->Texture)
				{
					for(auto OutputItr : GraphItr->OutputInstances)
					{
						if (Expression->Texture == OutputItr.Value->GetData())
						{
							SubstanceAir::OutputInstance* OutputFound = Substance::Helpers::GetSubstanceOutputByID(GraphItr, OutputItr.Key);

							if (OutputFound)
							{
								CurrentMaterialSet.Material = *MatItr;
								CurrentMaterialSet.ParameterNames.Add(OutputFound->mDesc.mIdentifier.c_str(), Expression);

								//Add all referencing material expressions to the update list
								ReferencingMaterials.Add(CurrentMaterialSet);
							}
						}
					}
				}
			}
		}

		TArray<MaterialInstanceParameterSet> ReferencingMaterialInstances;
		for (TObjectIterator<UMaterialInstance> MatItr; MatItr; ++MatItr)
		{
			MaterialInstanceParameterSet CurrentMaterialInstanceSet;
			for (int32 ParameterIndex = 0; ParameterIndex < MatItr->TextureParameterValues.Num(); ++ParameterIndex)
			{
				if (MatItr->TextureParameterValues[ParameterIndex].ParameterValue)
				{
					UTexture2D* SubstanceTexture = Cast<UTexture2D>(MatItr->TextureParameterValues[ParameterIndex].ParameterValue);
					for (auto OutputItr : GraphItr->OutputInstances)
					{
						if (SubstanceTexture == OutputItr.Value->GetData())
						{
							SubstanceAir::OutputInstance* OutputFound = Substance::Helpers::GetSubstanceOutputByID(GraphItr, OutputItr.Key); 

							if (OutputFound)
							{
								CurrentMaterialInstanceSet.MaterialInstance = *MatItr;
								CurrentMaterialInstanceSet.ParameterNames.Add(OutputFound->mDesc.mIdentifier.c_str(), MatItr->TextureParameterValues[ParameterIndex].ParameterInfo.Name);

								//Add all referencing material expressions to the update list
								ReferencingMaterialInstances.Add(CurrentMaterialInstanceSet);
							}
						}
					}
				}
			}
		}

		Substance::Helpers::Tick();

		//Array of all of the enabled output UIDs
		TMap<uint32, OutputTextureSettings> EnabledOutputs;

		//Save the enabled state of all of the outputs
		for (const auto& OutputItr : GraphItr->Instance->getOutputs())
		{
			if (OutputItr->mEnabled && OutputItr->mUserData && reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData)->GetData())
			{
				//Get the full path for the graph asset
				UTexture2D* Texture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData)->GetData());
				FString SanitizedPath;
				Texture->GetPathName().Split(TEXT("."), &(SanitizedPath), nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

				OutputTextureSettings SettingsBackup;
				SettingsBackup.PackageName = SanitizedPath;
				SettingsBackup.sRGB_Enabled = Texture->SRGB;

				EnabledOutputs.Add(OutputItr->mDesc.mUid, SettingsBackup);
			}
		}

		//Grab and store the presets
		SubstanceAir::shared_ptr<SubstanceAir::Preset> StorePreset = Substance::Helpers::EmptyPresetAsShared();
		SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> CurrentInstance = GraphItr->Instance;
		Substance::Helpers::CopyPresetData(CurrentInstance, *StorePreset.get());

		//Get the full path for the graph asset
		FString SanitizedPath;
		GraphItr->GetPathName().Split(TEXT("."), &(SanitizedPath), nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

		//Set all of the reference data needed for this graph
		ReimportData CurrentReimportData = ReimportData(GraphItr->PackageURL, SanitizedPath, GraphItr->CreatedMaterial,GraphItr->ConstantCreatedMaterial, StorePreset, ReferencingMaterials, ReferencingMaterialInstances, GraphItr->ImageSources, EnabledOutputs);
		PreviousInstanceTransfer.Add(CurrentReimportData);
	}
}
