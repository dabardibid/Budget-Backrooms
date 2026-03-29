// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceGraphInstance.cpp

#include "SubstanceGraphInstance.h"
#include "SubstanceCorePrivatePCH.h"
#include "SubstanceInstanceFactory.h"
#include "SubstanceCoreHelpers.h"
#include "SubstanceOutputData.h"
#include "SubstanceStructuresSerialization.h"
#include "SubstanceSettings.h"
#include "SubstanceTexture2D.h"

#include <substance/framework/input.h>
#include <substance/framework/package.h>
#include <substance/framework/preset.h>

#include "UObject/UObjectIterator.h"
#include "Containers/Map.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialLayersFunctions.h"
#include "Materials/MaterialExpressionOneMinus.h"

#if WITH_EDITOR
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "IAssetTools.h"
#include <AssetToolsModule.h>
#include "Serialization/ArchiveReplaceObjectRef.h"
#include "Misc/ScopedSlowTask.h"
#include "FileHelpers.h"
#include "AssetRegistryModule.h"
#endif


USubstanceGraphInstance::USubstanceGraphInstance(class FObjectInitializer const& PCIP) : Super(PCIP)
{
	bIsFrozen = false;
	Instance = nullptr;
	ConstantCreatedMaterial = nullptr;
	DynamicCreatedMaterial = nullptr;
	CreatedMaterial = nullptr;
	InstancePreset = MakeShared<SubstanceAir::Preset>();
}

void USubstanceGraphInstance::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	//If we aren't loading, always use most up to date serialization method
	if (!Ar.IsLoading())
	{
		Ar.UsingCustomVersion(FSubstanceCoreCustomVersion::GUID);
		SerializeCurrent(Ar);
		return;
	}

	//Check the version to see if we need to serialize legacy
	bool SerializeLegacy = false;
	if (Ar.CustomVer(FSubstanceCoreCustomVersion::GUID) < FSubstanceCoreCustomVersion::FrameworkRefactor)
	{
		//Handle Legacy Loading here
		SerializeLegacy = true;
	}

	//Register the most recent version with this AR
	Ar.UsingCustomVersion(FSubstanceCoreCustomVersion::GUID);

	//Call serialize based on version
	(SerializeLegacy == true) ? this->SerializeLegacy(Ar) : SerializeCurrent(Ar);
}

void USubstanceGraphInstance::SerializeCurrent(FArchive& Ar)
{
	//NOTE:: Used to debug serialization I/O position
	//UE_LOG(LogSubstanceCore, Warning, TEXT("Serialize Tell position: %d"), (int32)(Ar.TotalSize() - Ar.Tell()))

	//Save the instance index to link later
	Ar << PackageURL;

	//Load reference to the parent factory
	Ar << ParentFactory;

	if (Ar.IsSaving() && Instance)
	{
		SubstanceAir::Preset InstancePresets;

		//Make sure the instance hasn't been thrashed from and invalid parent factory before accessing it
		if (ParentFactory)
		{
			InstancePresets.fill(*Instance);
		}

		SubstanceAir::stringstream SS;
		SS << InstancePresets;

		SubstanceAir::string PresetString = SS.str();
		uint32 presetsSize = PresetString.length();
		Ar << presetsSize;

		Ar.ByteOrderSerialize((void*)PresetString.c_str(), presetsSize);

		//Associate this asset with the current plugin version
		Ar.UsingCustomVersion(FSubstanceCoreCustomVersion::GUID);
		Ar.SetCustomVersion(FSubstanceCoreCustomVersion::GUID, FSubstanceCoreCustomVersion::LatestVersion, FName("LegacyUpdated"));

		if (Ar.GetArchiveName() == FString("FDuplicateDataWriter"))
		{
			FString CurrentName;
			this->GetName(CurrentName);
			Ar << CurrentName;
		}
	}
	else if (Ar.IsLoading())
	{
		uint32 presetsSize = 0;
		Ar << presetsSize;

		if (presetsSize > 0)
		{
			char* presetData = new char[presetsSize];

			Ar.ByteOrderSerialize(presetData, presetsSize);

			SubstanceAir::parsePreset(*InstancePreset.Get(), presetData);
			delete[] presetData;

			//Handle transacting
			if (Ar.IsTransacting() && Instance && InstancePreset->mPackageUrl == Instance->mDesc.mPackageUrl)
			{
				InstancePreset->apply(*Instance);
			}
		}

		//Check to see if this is a duplicate reader - If so, read temp transfer data
		if (Ar.GetArchiveName() == FString("FDuplicateDataReader"))
		{
			FString SourceGraphName;
			Ar << SourceGraphName;

			//Find the source parent graph and assign it to the user data
			//NOTE:: This save is very temporary! This will be used in post duplicate and then will be overwritten soon after
			for (TObjectIterator<USubstanceGraphInstance> It; It; ++It)
			{
				if ((*It)->GetName() == SourceGraphName)
				{
					mUserData.ParentGraph = (*It);
					break;
				}
			}
		}
	}

	//Used to determine if this is a cooked build
	bool bCooked = Ar.IsCooking();

	//This is gross but it prevents a serialize size mismatch from a save corruption! (UE4-391)
	if ((!Ar.AtEnd() && Ar.IsLoading()) || !Ar.IsLoading())
	{
		Ar << bCooked;
	}
}

void USubstanceGraphInstance::SerializeLegacy(FArchive& Ar)
{
	//Serialize the old framework graph instance
	LegacySerailizeGraphInstance(Ar, this);

	//Serialize eventual fields from previous version
	if (Ar.IsLoading())
	{
		const int32 SbsCoreVer = GetLinkerCustomVersion(FSubstanceCoreCustomVersion::GUID);
		if (SbsCoreVer < FSubstanceCoreCustomVersion::FixedGraphFreeze)
		{
			int32 Padding;
			Ar << Padding;
			Ar << Padding;
		}
	}

	bool bCooked = Ar.IsCooking() && Ar.IsSaving();

	Ar << bCooked;
	Ar << ParentFactory;

	//Clear the archive to be updated with the new serialization on save.
	Ar.FlushCache();

	//Forcing package dirty
	UPackage* Package = GetOutermost();
	Package->SetDirtyFlag(true);
}

void USubstanceGraphInstance::CleanupGraphInstance()
{
	//Smoothly handle cleanup - clean up memory
	if (Instance)
	{
		//Reset Image Inputs
		Substance::Helpers::ResetGraphInstanceImageInputs(this);

		//Remove from queues and allows update to call delayed deletion.
		Substance::Helpers::ClearFromRender(this);

		//Disable all outputs
		for (const auto& ItOut : Instance->getOutputs())
		{
			Substance::Helpers::Disable(ItOut, false);
		}

		//Manually clear the smart pointer in the case this is called from the InstanceFactory and a package is being cleaned up
		Instance.reset();
	}
}

bool USubstanceGraphInstance::GraphRequiresUpdate()
{
#if WITH_EDITOR
	//Get a list of all of our graph instances
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().SearchAllAssets(true);
	TArray<FAssetData> AssetData;
	const UClass* Class = USubstanceTexture2D::StaticClass();
	AssetRegistryModule.Get().GetAssetsByClass(Class->GetFName(), AssetData);

	TArray<USubstanceTexture2D*> AllSubstanceTextures;
	for (auto& Itr : AssetData)
	{
		AllSubstanceTextures.AddUnique((USubstanceTexture2D*)Itr.GetAsset());
	}

	for (USubstanceTexture2D* TexItr : AllSubstanceTextures)
	{
		if (Instance->findOutput(TexItr->mUid) && TexItr->ParentInstance == this)
		{
			return true;
		}
	}
#endif
	return false;
}

void USubstanceGraphInstance::BeginDestroy()
{
	//Route BeginDestroy
	Super::BeginDestroy();

	//Smoothly handle cleanup - clean up memory
	CleanupGraphInstance();

	//Unregister the graph
	if (ParentFactory)
	{
		ParentFactory->UnregisterGraphInstance(this);
	}
}

void USubstanceGraphInstance::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	if (!bDuplicateForPIE)
	{
		//After duplication, we need to recreate a parent instance and set it as outer
		//we get this object from saving the name only on duplication
		USubstanceGraphInstance* RefInstance = mUserData.ParentGraph.Get();

		//Make sure the instance we copied from is valid!
		if (!RefInstance || !RefInstance->Instance)
		{
			UE_LOG(LogSubstanceCore, Error, TEXT("Can't setup graph after duplicating because the instance duplicated from is invalid!"))
			return;
		}

		//Make sure we can associate the new graph instance with the parent factory used to create the instance duplicated from
		if (!RefInstance->ParentFactory)
		{
			UE_LOG(LogSubstanceCore, Error, TEXT("Can't link the newly duplicated graph instance to a proper parent factory!"))
			return;
		}

		ParentFactory = RefInstance->ParentFactory;

		Instance = SubstanceAir::make_shared<SubstanceAir::GraphInstance>(RefInstance->Instance->mDesc);

		Substance::Helpers::CopyInstance(RefInstance, this, false);
		Substance::Helpers::CreateTextures(this);
		this->mUserData.ParentGraph = this;
		Instance->mUserData = (size_t)&mUserData;
#if WITH_EDITOR
		PrepareOutputsForSave();
		Substance::Helpers::RenderSync(Instance, true);
		SaveAllOutputs(true);
#else
		Substance::Helpers::RenderAsync(Instance);
#endif

#if WITH_EDITOR
		if (GIsEditor)
		{
			UObject* MaterialParent = nullptr;
			if (RefInstance->ConstantCreatedMaterial != nullptr)
			{
				CreatedMaterial = RefInstance->CreatedMaterial;
				FName NewMaterialName = MakeUniqueObjectName(RefInstance->ConstantCreatedMaterial->GetOuter(),
				                        RefInstance->ConstantCreatedMaterial->GetClass(), FName(*RefInstance->ConstantCreatedMaterial->GetName()));

				//Create the path to the material by getting the base path of the newly created instance;
				FString MaterialPath;
				FString CurrentFullPath = GetPathName();
				CurrentFullPath.Split(TEXT("/"), &(MaterialPath), nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

				FString FullAssetPath;
				FString AssetName;
				FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
				AssetToolsModule.Get().CreateUniqueAssetName(MaterialPath + TEXT("/") + NewMaterialName.ToString(), TEXT(""), FullAssetPath, AssetName);

				UObject* MaterialBasePackage = CreatePackage(*FullAssetPath);

				Substance::Helpers::CreateMaterial(this, AssetName, MaterialBasePackage);
			}

			TArray<UObject*> AssetList;
			for (auto itout : Instance->getOutputs())
			{
				//Skip unsupported outputs
				if (itout->mUserData == 0)
				{
					continue;
				}

				AssetList.AddUnique(reinterpret_cast<USubstanceOutputData*>(itout->mUserData)->GetData());
			}
			AssetList.AddUnique(this);
			if (ConstantCreatedMaterial)
			{
				AssetList.AddUnique(ConstantCreatedMaterial);
				UE_LOG(LogSubstanceCore, Warning, TEXT("Material Duplicated for - %s. Should now be added to asset list. Material Name - %s"),
				       *GetName(), *ConstantCreatedMaterial->GetName());
			}

			FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			ContentBrowserModule.Get().SyncBrowserToAssets(AssetList);
		}
#endif
	}
}

void USubstanceGraphInstance::PostLoad()
{
	//Make sure that the factory that created this is valid
	if (!ParentFactory)
	{
		UE_LOG(LogSubstanceCore, Log, TEXT("Impossible to find parent package for \"%s\". All outputs resorting to baked."), *GetName());
		Super::PostLoad();
		return;
	}

	//Make sure the parent factory is loaded
	ParentFactory->ConditionalPostLoad();


	if (Instance == nullptr)
	{
		//Link the instance to one previously loaded
		if (this->LinkLoadedGraphInstance() == false)
		{
			Super::PostLoad();
			return;
		}

		//Instantiate the user data
		mUserData.ParentGraph = this;
		mUserData.bHasPendingImageInputRendering = true;
		Instance->mUserData = (size_t)&mUserData;

		//Only finalize on legacy load
		if (InstancePreset->mPackageUrl.empty())
		{
			InstancePreset->mPackageUrl = Instance->mDesc.mPackageUrl;
			FinalizeLegacyPresets(this);
		}

		//Set instance presets
		if (InstancePreset->mPackageUrl == Instance->mDesc.mPackageUrl)
		{
			if (!InstancePreset->apply(*Instance) && !InstancePreset->mInputValues.empty())
			{
				UE_LOG(LogSubstanceCore, Warning, TEXT("Failed to apply presets for instance - %s."), *GetName());
			}
		}
	}

	//The input instances with the graph and their desc
	for (const auto& ItInInst : Instance->getInputs())
	{
		if (ItInInst->mDesc.isImage())
		{
			SubstanceAir::InputInstanceImage* ImgInput = (SubstanceAir::InputInstanceImage*)ItInInst;
			if (!LinkImageInput(ImgInput))
			{
				FString InputLabel(ItInInst->mDesc.mLabel.c_str());
				UE_LOG(LogSubstanceCore, Warning, TEXT("Failed to find Image Input UAsset for - %s."), *InputLabel);
			}
		}
	}

	//Let the substance texture switch outputs back on
	for (const auto& ItOut : Instance->getOutputs())
	{
		Substance::Helpers::OverwriteSubstancePixelFormatForRuntimeCompression(ItOut);

		USubstanceOutputData** data = OutputInstances.Find(ItOut->mDesc.mUid);
		if(data != nullptr)
			ItOut->mUserData = (size_t)*data;
	}

	//Register this graph with the parent factory
	ParentFactory->RegisterGraphInstance(this);

	Super::PostLoad();
}

#if WITH_EDITOR
bool USubstanceGraphInstance::CanEditChange(const FProperty* InProperty) const
{
	return true;
}

void USubstanceGraphInstance::PostEditUndo()
{
	Substance::Helpers::RenderAsync(Instance);
}
void USubstanceGraphInstance::PreSave(const ITargetPlatform* TargetPlatform)
{
	Super::PreSave(TargetPlatform);
}

void USubstanceGraphInstance::PrepareOutputsForSave()
{
	for (auto& OutputDataItr : OutputInstances)
	{
		((USubstanceOutputData*)OutputDataItr.Value)->MarkPackageDirty();
	}

	Substance::Helpers::ClearFromRender(this);

	for (const auto& ItOut : Instance->getOutputs())
	{
		//Force outputs to source format
		Substance::Helpers::OverwriteSubstancePixelFormatForSourceImage(ItOut);
		ItOut->flagAsDirty();
	}
}

void USubstanceGraphInstance::SaveAllOutputs(bool ForceSave)
{
	if (ForceSave)
	{
		TArray<UPackage*> ThingsToSave;
		for (const auto& OutputItr : OutputInstances)
		{
			if (OutputItr.Value->GetData())
			{
				ThingsToSave.Add((UPackage*)OutputItr.Value->GetData()->GetOuter());
			}
		}

		ThingsToSave.Add((UPackage*)this->GetOuter());
		if (ParentFactory)
			ThingsToSave.Add((UPackage*)ParentFactory->GetOuter());
		if (ConstantCreatedMaterial)
			ThingsToSave.Add((UPackage*)ConstantCreatedMaterial->GetOuter());

		FEditorFileUtils::SaveDirtyPackages(false, false, true, false, false, false);
	}

	for (const auto& ItOut : Instance->getOutputs())
	{
		//Force outputs back to valid runtime formats
		Substance::Helpers::OverwriteSubstancePixelFormatForRuntimeCompression(ItOut);
	}
}
#endif //WITH_EDITOR

void USubstanceGraphInstance::RenderSync()
{
	Substance::Helpers::RenderSync(Instance, true);
}

bool USubstanceGraphInstance::CanUpdate()
{
	if (bIsFrozen)
	{
		UE_LOG(LogSubstanceCore, Warning, TEXT("Cannot modify Graph Instance \"%s\", instance is frozen."),
		       *GetName());
		return false;
	}

	return true;
}

bool USubstanceGraphInstance::SetInputImg(const FString& Name, class UObject* Value)
{
	if (Instance)
	{
		if (Substance::Helpers::IsSupportedImageInput(Value))
		{
			UObject* PrevSource = GetInputImg(Name);

			SubstanceAir::InputInstanceBase* CurrentInputImage = nullptr;

			for (const auto& ItInInst : Instance->getInputs())
			{
				if (ItInInst->mDesc.mIdentifier.c_str() == Name && ItInInst->mDesc.mType == Substance_IOType_Image)
				{
					CurrentInputImage = ItInInst;
				}
			}

			if (CurrentInputImage)
			{
				Substance::Helpers::UpdateInput(this->Instance, CurrentInputImage, Value);
			}

			bool bUseOtherInput = false;
			if (PrevSource)
			{
				for (auto InputName : GetInputNames())
				{
					if (PrevSource == GetInputImg(InputName))
					{
						bUseOtherInput = true;
					}
				}
			}

			UTexture2D* PrevImageInput = Cast<UTexture2D>(PrevSource);
			UTexture2D* NewImageInput = Cast<UTexture2D>(Value);
			if (!bUseOtherInput)
			{
				const uint32* Key = ImageSources.FindKey(PrevImageInput);
				if (Key)
				{
					ImageSources[*Key] = NewImageInput;
				}
			}

			Substance::Helpers::RenderAsync(Instance);
			return true;
		}
	}

	return false;
}

class UObject* USubstanceGraphInstance::GetInputImg(const FString& Name)
{
	for (const auto& ItInInst : Instance->getInputs())
	{
		if (ItInInst->mDesc.mIdentifier.c_str() == Name && ItInInst->mDesc.mType == Substance_IOType_Image)
		{
			SubstanceAir::InputInstanceImage* TypedInst = (SubstanceAir::InputInstanceImage*)ItInInst;

			if (TypedInst->getImage())
			{
				UObject* CurrentSource = reinterpret_cast<UObject*>(TypedInst->getImage()->mUserData);
				return CurrentSource;
			}
		}
	}

	return nullptr;
}

UMaterialInstanceConstant* USubstanceGraphInstance::GetConstantMaterial() {
	return ConstantCreatedMaterial;
}

void USubstanceGraphInstance::CreateMaterial(const FString& PackageName, UMaterial* ParentMaterial)
{
	#if WITH_EDITOR
	CreatedMaterial = ParentMaterial;
	FString SanitizedPackageName = UPackageTools::SanitizePackageName(PackageName);
	FString ObjectName;
	SanitizedPackageName.Split(TEXT("/"), nullptr, &(ObjectName), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	UObject* BasePackage = CreatePackage(*SanitizedPackageName);
	Substance::Helpers::CreateMaterial(this, ObjectName, BasePackage);
	#endif
}

UMaterialInstanceDynamic* USubstanceGraphInstance::GetDynamicMaterialInstance(FName Name, UMaterial* InParentMaterial)
{
	if (DynamicCreatedMaterial == nullptr)
	{
		if (InParentMaterial == nullptr)
			InParentMaterial = CreatedMaterial;

		FName InName = Name;

		if (InName == "None")
		{
			FString string;
			
			GetName(string);

			string.Append("_DynMat");

			InName = FName(*string);
			
		}

		DynamicCreatedMaterial = UMaterialInstanceDynamic::Create(InParentMaterial, NULL, InName);

		Substance::Helpers::GenerateMaterialExpressions(Instance.get(), DynamicCreatedMaterial, this);
	}

	return DynamicCreatedMaterial;
}

TArray<FString> USubstanceGraphInstance::GetInputNames()
{
	TArray<FString> Names;

	for (const auto& ItInInst : Instance->getInputs())
	{
		Names.Add(ItInInst->mDesc.mIdentifier.c_str());
	}

	return Names;
}

ESubstanceInputType USubstanceGraphInstance::GetInputType(FString InputName)
{
	for (const auto& ItInInst : Instance->getInputs())
	{
		if (ItInInst->mDesc.mIdentifier.c_str() == InputName)
		{
			return (ESubstanceInputType)(ItInInst->mDesc.mType);
		}
	}

	return ESubstanceInputType::SIT_MAX;
}

void USubstanceGraphInstance::SetInputInt(FString IntputName, const TArray<int32>& InputValues)
{
	if (Instance)
	{
		//Loop through our inputs and try and find a match
		for (const auto& CurrentInput : Instance->getInputs())
		{
			if (CurrentInput->mDesc.mIdentifier.c_str() == IntputName && CurrentInput->mDesc.isNumerical())
			{
				Substance::Helpers::SetNumericalInputValue((SubstanceAir::InputInstanceNumericalBase*)CurrentInput, InputValues);
				Substance::Helpers::UpdateInput(this->Instance, CurrentInput, InputValues);
				break;
			}
		}
	}
}

void USubstanceGraphInstance::SetInputFloat(FString IntputName, const TArray<float>& InputValues)
{
	if (Instance)
	{
		//Loop through our inputs and try and find a match
		for (const auto& CurrentInput : Instance->getInputs())
		{
			if (CurrentInput->mDesc.mIdentifier.c_str() == IntputName && CurrentInput->mDesc.isNumerical())
			{
				Substance::Helpers::SetNumericalInputValue((SubstanceAir::InputInstanceNumericalBase*)CurrentInput, InputValues);
				Substance::Helpers::UpdateInput(this->Instance, CurrentInput, InputValues);
				break;
			}
		}
	}
}

void USubstanceGraphInstance::SetInputString(FString Identifier, const FString& InputValue)
{
	if (Instance)
	{
		//Loop through our inputs and try and find a match
		for (const auto& CurrentInput : Instance->getInputs())
		{
			if (CurrentInput->mDesc.mIdentifier.c_str() == Identifier && CurrentInput->mDesc.isString())
			{
				Substance::Helpers::SetStringInputValue(CurrentInput, InputValue);
				Substance::Helpers::UpdateInput(this->Instance, CurrentInput, InputValue);
				break;
			}
		}
	}
}

void USubstanceGraphInstance::SetInputColor(FString Identifier, const FLinearColor& InputValue)
{
	//Format value in a way we can pass to the engine
	TArray<float> InputValues;
	InputValues.Add(InputValue.R);
	InputValues.Add(InputValue.G);
	InputValues.Add(InputValue.B);
	InputValues.Add(InputValue.A);

	if (Instance)
	{
		//Loop through our inputs and try and find a match
		for (const auto& CurrentInput : Instance->getInputs())
		{
			if (CurrentInput->mDesc.mIdentifier.c_str() == Identifier && CurrentInput->mDesc.isNumerical())
			{
				Substance::Helpers::SetNumericalInputValue((SubstanceAir::InputInstanceNumericalBase*)CurrentInput, InputValues);
				Substance::Helpers::UpdateInput(this->Instance, CurrentInput, InputValues);
				break;
			}
		}
	}
}

void USubstanceGraphInstance::SetInputBool(FString Identifier, bool Bool)
{
	//Format value in a way we can pass to the engine
	TArray<int32> Values;
	Values.Add((int32)Bool);

	if (Instance)
	{
		//Loop through our inputs and try and find a match
		for (const auto& CurrentInput : Instance->getInputs())
		{
			if (CurrentInput->mDesc.mIdentifier.c_str() == Identifier && CurrentInput->mDesc.isNumerical())
			{
				Substance::Helpers::SetNumericalInputValue((SubstanceAir::InputInstanceNumericalBase*)CurrentInput, Values);
				Substance::Helpers::UpdateInput(this->Instance, CurrentInput, Values);
				break;
			}
		}
	}
}

TArray<int32> USubstanceGraphInstance::GetInputInt(FString IntputName)
{
	TArray<int32> DummyValue;

	for (const auto& ItInInst : Instance->getInputs())
	{
		if (ItInInst->mDesc.mIdentifier.c_str() == IntputName &&
		        (ItInInst->mDesc.mType == Substance_IOType_Integer ||
		         ItInInst->mDesc.mType == Substance_IOType_Integer2 ||
		         ItInInst->mDesc.mType == Substance_IOType_Integer3 ||
		         ItInInst->mDesc.mType == Substance_IOType_Integer4))
		{
			return Substance::Helpers::GetValueInt(*ItInInst);
		}
	}

	return DummyValue;
}

TArray<float> USubstanceGraphInstance::GetInputFloat(FString IntputName)
{
	TArray<float> DummyValue;

	for (const auto& ItInInst : Instance->getInputs())
	{
		if (ItInInst->mDesc.mIdentifier.c_str() == IntputName &&
		        (ItInInst->mDesc.mType == Substance_IOType_Float ||
		         ItInInst->mDesc.mType == Substance_IOType_Float2 ||
		         ItInInst->mDesc.mType == Substance_IOType_Float3 ||
		         ItInInst->mDesc.mType == Substance_IOType_Float4))
		{
			return Substance::Helpers::GetValueFloat(*ItInInst);
		}
	}

	return DummyValue;
}

FString USubstanceGraphInstance::GetInputString(FString Identifier)
{
	for (const auto& ItInInst : Instance->getInputs())
	{
		if (ItInInst->mDesc.mIdentifier.c_str() == Identifier && ItInInst->mDesc.mType == Substance_IOType_String)
		{
			return Substance::Helpers::GetValueString(*ItInInst);
		}
	}

	UE_LOG(LogSubstanceCore, Warning, TEXT("Could not find a string input value - returning emptry string value"))
	return FString();
}

FLinearColor USubstanceGraphInstance::GetInputColor(FString Identifier)
{
	for (const auto& ItInInst : Instance->getInputs())
	{
		if (ItInInst->mDesc.mIdentifier.c_str() == Identifier && (ItInInst->mDesc.mType == Substance_IOType_Float4 || ItInInst->mDesc.mType == Substance_IOType_Float3))
		{
			FLinearColor ReturnColor;
			TArray<float> Values = Substance::Helpers::GetValueFloat(*ItInInst);

			ReturnColor.R = Values[0];
			ReturnColor.G = Values[1];
			ReturnColor.B = Values[2];

			if (Values.Num() > 3)
			{
				ReturnColor.A = Values[3];
			}
			else
			{
				ReturnColor.A = 1.0f;
			}

			return ReturnColor;
		}
	}

	UE_LOG(LogSubstanceCore, Warning, TEXT("Could not find a Color input value - returning emptry Color value"))
	return FLinearColor();
}

bool USubstanceGraphInstance::GetInputBool(FString Identifier)
{
	for (const auto& ItInInst : Instance->getInputs())
	{
		if (ItInInst->mDesc.mIdentifier.c_str() == Identifier && ItInInst->mDesc.mType == Substance_IOType_String)
		{
			int32 Value = Substance::Helpers::GetValueInt(*ItInInst)[0];
			return (Value > 0);
		}
	}

	UE_LOG(LogSubstanceCore, Warning, TEXT("Could not find a bool input value - returning true value"))
	return true;
}

using namespace Substance;

FSubstanceFloatInputDesc USubstanceGraphInstance::GetFloatInputDesc(FString IntputName)
{
	using namespace SubstanceAir;
	FSubstanceFloatInputDesc K2_InputDesc;

	for (const auto& ItInInst : Instance->getInputs())
	{
		if (ItInInst->mDesc.mIdentifier.c_str() == IntputName &&
		        (ItInInst->mDesc.mType == Substance_IOType_Float ||
		         ItInInst->mDesc.mType == Substance_IOType_Float2 ||
		         ItInInst->mDesc.mType == Substance_IOType_Float3 ||
		         ItInInst->mDesc.mType == Substance_IOType_Float4))
		{
			const SubstanceAir::InputDescBase* InputDesc = &ItInInst->mDesc;

			K2_InputDesc.Name = IntputName;

			switch (ItInInst->mDesc.mType)
			{
			case Substance_IOType_Float:
				K2_InputDesc.Min.Add(((InputDescFloat*)InputDesc)->mMinValue);
				K2_InputDesc.Max.Add(((InputDescFloat*)InputDesc)->mMaxValue);
				K2_InputDesc.Default.Add(((InputDescFloat*)InputDesc)->mDefaultValue);
				break;
			case Substance_IOType_Float2:
				K2_InputDesc.Min.Add(((InputDescFloat2*)InputDesc)->mMinValue.x);
				K2_InputDesc.Min.Add(((InputDescFloat2*)InputDesc)->mMinValue.y);

				K2_InputDesc.Max.Add(((InputDescFloat2*)InputDesc)->mMaxValue.x);
				K2_InputDesc.Max.Add(((InputDescFloat2*)InputDesc)->mMaxValue.y);

				K2_InputDesc.Default.Add(((InputDescFloat2*)InputDesc)->mDefaultValue.x);
				K2_InputDesc.Default.Add(((InputDescFloat2*)InputDesc)->mDefaultValue.y);
				break;
			case Substance_IOType_Float3:
				K2_InputDesc.Min.Add(((InputDescFloat3*)InputDesc)->mMinValue.x);
				K2_InputDesc.Min.Add(((InputDescFloat3*)InputDesc)->mMinValue.y);
				K2_InputDesc.Min.Add(((InputDescFloat3*)InputDesc)->mMinValue.z);

				K2_InputDesc.Max.Add(((InputDescFloat3*)InputDesc)->mMaxValue.x);
				K2_InputDesc.Max.Add(((InputDescFloat3*)InputDesc)->mMaxValue.y);
				K2_InputDesc.Max.Add(((InputDescFloat3*)InputDesc)->mMaxValue.z);

				K2_InputDesc.Default.Add(((InputDescFloat3*)InputDesc)->mDefaultValue.x);
				K2_InputDesc.Default.Add(((InputDescFloat3*)InputDesc)->mDefaultValue.y);
				K2_InputDesc.Default.Add(((InputDescFloat3*)InputDesc)->mDefaultValue.z);
				break;
			case Substance_IOType_Float4:
				K2_InputDesc.Min.Add(((InputDescFloat4*)InputDesc)->mMinValue.x);
				K2_InputDesc.Min.Add(((InputDescFloat4*)InputDesc)->mMinValue.y);
				K2_InputDesc.Min.Add(((InputDescFloat4*)InputDesc)->mMinValue.z);
				K2_InputDesc.Min.Add(((InputDescFloat4*)InputDesc)->mMinValue.w);

				K2_InputDesc.Max.Add(((InputDescFloat4*)InputDesc)->mMaxValue.x);
				K2_InputDesc.Max.Add(((InputDescFloat4*)InputDesc)->mMaxValue.y);
				K2_InputDesc.Max.Add(((InputDescFloat4*)InputDesc)->mMaxValue.z);
				K2_InputDesc.Min.Add(((InputDescFloat4*)InputDesc)->mMaxValue.w);

				K2_InputDesc.Default.Add(((InputDescFloat4*)InputDesc)->mDefaultValue.x);
				K2_InputDesc.Default.Add(((InputDescFloat4*)InputDesc)->mDefaultValue.y);
				K2_InputDesc.Default.Add(((InputDescFloat4*)InputDesc)->mDefaultValue.z);
				K2_InputDesc.Default.Add(((InputDescFloat4*)InputDesc)->mDefaultValue.w);
				break;
			}
		}
	}

	return K2_InputDesc;
}

FSubstanceIntInputDesc USubstanceGraphInstance::GetIntInputDesc(FString IntputName)
{
	using namespace SubstanceAir;
	FSubstanceIntInputDesc K2_InputDesc;

	for (const auto& ItInInst : Instance->getInputs())
	{
		if (ItInInst->mDesc.mIdentifier.c_str() == IntputName &&
		        (ItInInst->mDesc.mType == Substance_IOType_Integer ||
		         ItInInst->mDesc.mType == Substance_IOType_Integer2 ||
		         ItInInst->mDesc.mType == Substance_IOType_Integer3 ||
		         ItInInst->mDesc.mType == Substance_IOType_Integer4))
		{
			const SubstanceAir::InputDescBase* InputDesc = &ItInInst->mDesc;

			K2_InputDesc.Name = IntputName;

			switch (ItInInst->mDesc.mType)
			{
			case Substance_IOType_Integer:
				K2_InputDesc.Min.Add(((SubstanceAir::InputDescInt*)InputDesc)->mMinValue);
				K2_InputDesc.Max.Add(((SubstanceAir::InputDescInt*)InputDesc)->mMaxValue);
				K2_InputDesc.Default.Add(((SubstanceAir::InputDescInt*)InputDesc)->mDefaultValue);
				break;
			case Substance_IOType_Integer2:
				K2_InputDesc.Min.Add(((SubstanceAir::InputDescInt2*)InputDesc)->mMinValue.x);
				K2_InputDesc.Min.Add(((SubstanceAir::InputDescInt2*)InputDesc)->mMinValue.y);

				K2_InputDesc.Max.Add(((SubstanceAir::InputDescInt2*)InputDesc)->mMaxValue.x);
				K2_InputDesc.Max.Add(((SubstanceAir::InputDescInt2*)InputDesc)->mMaxValue.y);

				K2_InputDesc.Default.Add(((SubstanceAir::InputDescInt2*)InputDesc)->mDefaultValue.x);
				K2_InputDesc.Default.Add(((SubstanceAir::InputDescInt2*)InputDesc)->mDefaultValue.y);
				break;
			case Substance_IOType_Integer3:
				K2_InputDesc.Min.Add(((SubstanceAir::InputDescInt3*)InputDesc)->mMinValue.x);
				K2_InputDesc.Min.Add(((SubstanceAir::InputDescInt3*)InputDesc)->mMinValue.y);
				K2_InputDesc.Min.Add(((SubstanceAir::InputDescInt3*)InputDesc)->mMinValue.z);

				K2_InputDesc.Max.Add(((SubstanceAir::InputDescInt3*)InputDesc)->mMaxValue.x);
				K2_InputDesc.Max.Add(((SubstanceAir::InputDescInt3*)InputDesc)->mMaxValue.y);
				K2_InputDesc.Max.Add(((SubstanceAir::InputDescInt3*)InputDesc)->mMaxValue.z);

				K2_InputDesc.Default.Add(((SubstanceAir::InputDescInt3*)InputDesc)->mDefaultValue.x);
				K2_InputDesc.Default.Add(((SubstanceAir::InputDescInt3*)InputDesc)->mDefaultValue.y);
				K2_InputDesc.Default.Add(((SubstanceAir::InputDescInt3*)InputDesc)->mDefaultValue.z);
				break;
			case Substance_IOType_Integer4:
				K2_InputDesc.Min.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mMinValue.x);
				K2_InputDesc.Min.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mMinValue.y);
				K2_InputDesc.Min.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mMinValue.z);
				K2_InputDesc.Min.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mMinValue.w);

				K2_InputDesc.Max.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mMaxValue.x);
				K2_InputDesc.Max.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mMaxValue.y);
				K2_InputDesc.Max.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mMaxValue.z);
				K2_InputDesc.Max.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mMaxValue.w);

				K2_InputDesc.Default.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mDefaultValue.x);
				K2_InputDesc.Default.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mDefaultValue.y);
				K2_InputDesc.Default.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mDefaultValue.z);
				K2_InputDesc.Default.Add(((SubstanceAir::InputDescInt4*)InputDesc)->mDefaultValue.w);
				break;
			}
		}
	}

	return K2_InputDesc;
}

FSubstanceInstanceDesc USubstanceGraphInstance::GetInstanceDesc()
{
	FSubstanceInstanceDesc K2_InstanceDesc;
	K2_InstanceDesc.Name = Instance->mDesc.mLabel.c_str();
	return K2_InstanceDesc;
}


TArray<FString> USubstanceGraphInstance::GetOutputNames()
{
	const auto outputs = Instance->getOutputs();
	TArray<FString> names;
	names.SetNum(outputs.size(), false);
	for (int i = 0; i < outputs.size(); i++)
	{
		names[i] = FString(outputs[i]->mDesc.mIdentifier.c_str());
	}
	return names;
}

void USubstanceGraphInstance::EnableOutput(FString Identifier, bool Value)
{
	for (const auto& output : Instance->getOutputs())
	{
		if (output->mDesc.mIdentifier.c_str() == Identifier)
		{
			output->mEnabled = Value;
		}
	}
}


void USubstanceGraphInstance::CreateOutputs()
{
	Substance::Helpers::CreateTextures(this);
	#if WITH_EDITORONLY_DATA
	for (auto output : Instance->getOutputs())
	{
		Substance::Helpers::OverwriteSubstancePixelFormatForSourceImage(output);
	}
	#endif
	MarkPackageDirty();
	TArray<int32> DefaultOutputSize{ GetDefault<USubstanceSettings>()->DefaultSubstanceOutputSizeX, GetDefault<USubstanceSettings>()->DefaultSubstanceOutputSizeY };
	SetInputInt("$outputsize", DefaultOutputSize);
}


bool USubstanceGraphInstance::LinkLoadedGraphInstance()
{
	for (const auto& Itr : ParentFactory->SubstancePackage->getGraphs())
	{
		if (Itr.mPackageUrl.c_str() == PackageURL)
		{
			Instance = SubstanceAir::make_shared<SubstanceAir::GraphInstance>(Itr);
			break;
		}
	}

	if (Instance == nullptr)
	{
		UE_LOG(LogSubstanceCore, Warning, TEXT("Could not find a matching desc to for graph instance %s"), *GetName());
		return false;
	}

	return true;
}

bool USubstanceGraphInstance::LinkImageInput(SubstanceAir::InputInstanceImage* ImageInput)
{
	//Check the map to see if we have the input
	UTexture2D* SubstanceImage = nullptr;
	if (ImageSources.Find(ImageInput->mDesc.mUid))
	{
		SubstanceImage = *ImageSources.Find(ImageInput->mDesc.mUid);
	}

	if (SubstanceImage == nullptr)
	{
		return false;
	}

	//Sets the image input
	Substance::Helpers::SetImageInput(ImageInput, (UObject*)SubstanceImage, Instance);

	//Make sure the image is loaded
	SubstanceImage->ConditionalPostLoad();

	//Delay the set image input, the source is not necessarily ready
	Substance::Helpers::PushDelayedImageInput(ImageInput, Instance);
	mUserData.bHasPendingImageInputRendering = true;

	return true;
}


USubstanceGraphInstance* USubstanceGraphInstance::Duplicate()
{
	return Substance::Helpers::DuplicateGraphInstance(this);
}