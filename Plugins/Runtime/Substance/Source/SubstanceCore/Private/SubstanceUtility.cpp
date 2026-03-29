// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceUtility.cpp

#include "SubstanceUtility.h"
#include "SubstanceCorePrivatePCH.h"
#include "SubstanceGraphInstance.h"
#include "SubstanceInstanceFactory.h"
#include "SubstanceCoreHelpers.h"
#include "SubstanceSettings.h"
#include "SubstanceOutputData.h"
#include "substance/framework/framework.h"

#include "Engine/Texture2D.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "UObject/UObjectIterator.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITORONLY_DATA
#include "SubstanceCoreModule.h"
#endif

USubstanceUtility::USubstanceUtility(class FObjectInitializer const& PCIP) : Super(PCIP)
{
}

TArray<class USubstanceGraphInstance*> USubstanceUtility::GetSubstances(class UMaterialInterface* MaterialInterface)
{
	TArray<class USubstanceGraphInstance*> Substances;

	if (!MaterialInterface)
	{
		return Substances;
	}

	UMaterial* Material = MaterialInterface->GetMaterial();

	
    for (TObjectIterator<USubstanceGraphInstance> Itr; Itr; ++Itr)
    {
        USubstanceGraphInstance* graph = *Itr;
        if (graph->ConstantCreatedMaterial->GetMaterial() == Material)
        {
            Substances.AddUnique(graph);
        }
    }

	return Substances;
}

TArray<class UTexture2D*> USubstanceUtility::GetSubstanceTextures(class USubstanceGraphInstance* GraphInstance)
{
	TArray<class UTexture2D*> SubstanceTextures;

	if (!GraphInstance)
	{
		return SubstanceTextures;
	}

	for (const auto& OutputItr : GraphInstance->Instance->getOutputs())
	{
		if (OutputItr->mEnabled && OutputItr->mUserData != 0)
		{
			SubstanceTextures.Add(Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData)->GetData()));
		}
	}
	return SubstanceTextures;
}

FString USubstanceUtility::GetGraphName(USubstanceGraphInstance* GraphInstance)
{
	FString GraphInstanceName;
	if (GraphInstance)
		GraphInstanceName = GraphInstance->GetInstanceDesc().Name;
	return GraphInstanceName;
}

FString USubstanceUtility::GetFactoryName(USubstanceGraphInstance* GraphInstance)
{
	FString ParentFactoryName;
	if (GraphInstance && GraphInstance->ParentFactory)
		ParentFactoryName = GraphInstance->ParentFactory->GetFName().ToString();
	return ParentFactoryName;
}

float USubstanceUtility::GetSubstanceLoadingProgress()
{
	return Substance::Helpers::GetSubstanceLoadingProgress();
}

USubstanceGraphInstance* USubstanceUtility::CreateGraphInstance(UObject* WorldContextObject, USubstanceInstanceFactory* Factory, int32 GraphDescIndex, UMaterial* ParentMaterial, FString InstanceName)
{
	if (!WorldContextObject || !ParentMaterial)
		return nullptr;

	USubstanceGraphInstance* GraphInstance = nullptr;

	if (Factory && Factory->SubstancePackage && GraphDescIndex < (int32)Factory->SubstancePackage->getGraphs().size())
	{

		//Set package parent
		UObject* Outer = WorldContextObject ? WorldContextObject : GetTransientPackage();

		//Copy of instantiate
		GraphInstance = NewObject<USubstanceGraphInstance>(Outer, *InstanceName, RF_Transient);

		//Register factory used to create this
		GraphInstance->ParentFactory = Factory;
		Factory->RegisterGraphInstance(GraphInstance);

		//Get a reference to the graph desc used to create our instance
		const SubstanceAir::GraphDesc& Desc = Factory->SubstancePackage->getGraphs()[GraphDescIndex];

		//Create a new instance
		GraphInstance->Instance = SubstanceAir::make_shared<SubstanceAir::GraphInstance>(Desc);
		
#if !WITH_EDITORONLY_DATA
		for (const auto& ItOut : GraphInstance->Instance->getOutputs())
		{
			//Force outputs back to valid runtime formats
			Substance::Helpers::OverwriteSubstancePixelFormatForRuntimeCompression(ItOut);
		}
#else
		FSubstanceCoreModule& SubstanceModule = FModuleManager::LoadModuleChecked<FSubstanceCoreModule>(TEXT("SubstanceCore"));
		const bool PIE = SubstanceModule.isPie();
		
		if (PIE)
		{
			for (const auto& ItOut : GraphInstance->Instance->getOutputs())
			{
				//Force outputs back to valid runtime formats
				Substance::Helpers::OverwriteSubstancePixelFormatForRuntimeCompression(ItOut);
			}
		}
#endif

		//Set up the user data for framework access
		GraphInstance->mUserData.ParentGraph = GraphInstance;
		GraphInstance->Instance->mUserData = (size_t)&GraphInstance->mUserData;
		GraphInstance->CreatedMaterial = ParentMaterial;
		//Create textures that will not be saved
		Substance::Helpers::CreateTextures(GraphInstance, true, true);

		//Initial update
#if WITH_EDITORONLY_DATA
		if (!PIE)
		{
			GraphInstance->PrepareOutputsForSave();
		}

		Substance::Helpers::RenderSync(GraphInstance->Instance, true);

		if (!PIE)
		{
			GraphInstance->SaveAllOutputs(true);
		}
#endif
	}

	return GraphInstance;
}

USubstanceGraphInstance* USubstanceUtility::DuplicateGraphInstance(UObject* WorldContextObject, USubstanceGraphInstance* GraphInstance)
{
	check(WorldContextObject);

	USubstanceGraphInstance* NewGraphInstance = nullptr;

	if (GraphInstance && GraphInstance->ParentFactory && GraphInstance->ParentFactory->SubstancePackage)
	{
		int idx = 0;
		for (const auto& itGraph : GraphInstance->ParentFactory->SubstancePackage->getGraphs())
		{
			if (GraphInstance->Instance->mDesc.mParent->getUid() == itGraph.mParent->getUid())
			{
				break;
			}
			++idx;
		}

		NewGraphInstance = CreateGraphInstance(WorldContextObject, GraphInstance->ParentFactory, idx, GraphInstance->CreatedMaterial);
		CopyInputParameters(GraphInstance, NewGraphInstance);
	}

	return NewGraphInstance;
}

//NOTE:: For disable and enable outputs, passing the indices is not clear from a user stand point. There is no way to easily see which output is associated
//with each index. It would be beneficial in the long run to try and find a way pass a different parameter to enable / disable an output.
void USubstanceUtility::EnableInstanceOutputs(UObject* WorldContextObject, USubstanceGraphInstance* GraphInstance, TArray<int32> OutputIndices)
{
	if (!GraphInstance || !GraphInstance->Instance)
	{
		return;
	}

	for (const auto& IdxIt : OutputIndices)
	{
		if (IdxIt >= (int32)GraphInstance->Instance->getOutputs().size())
		{
			UE_LOG(LogSubstanceCore, Warning, TEXT("Invalid Output Index passed for %s, index %d out of range!"), *GraphInstance->GetName(), IdxIt);
			continue;
		}

		SubstanceAir::OutputInstance* OutputInst = GraphInstance->Instance->getOutputs()[IdxIt];

		//Skip unsupported texture formats
		if (OutputInst->mUserData == 0)
		{
			Substance::Helpers::CreateSubstanceTexture2D(OutputInst, false, FString(), WorldContextObject ? WorldContextObject : GetTransientPackage());
		}
	}

	//Upload some place holder content in the texture to make the texture usable
	Substance::Helpers::CreatePlaceHolderMips(GraphInstance->Instance);
}

void USubstanceUtility::DisableInstanceOutputs(UObject* WorldContextObject, USubstanceGraphInstance* GraphInstance, TArray<int32> OutputIndices)
{
	if (!GraphInstance || !GraphInstance->Instance)
	{
		return;
	}

	for (const auto& IdxIt : OutputIndices)
	{
		if (IdxIt >= (int32)GraphInstance->Instance->getOutputs().size())
		{
			UE_LOG(LogSubstanceCore, Warning, TEXT("Invalid Output Index passed for %s, index %d out of range!"), *GraphInstance->GetName(), IdxIt);
			continue;
		}

		SubstanceAir::OutputInstance* OutputInst = GraphInstance->Instance->getOutputs()[IdxIt];
		Substance::Helpers::Disable(OutputInst);
	}
}

void USubstanceUtility::CopyInputParameters(USubstanceGraphInstance* SourceGraphInstance, USubstanceGraphInstance* DestGraphInstance)
{
	if (!SourceGraphInstance || !SourceGraphInstance->Instance || !DestGraphInstance || !DestGraphInstance->Instance)
	{
		return;
	}

	SubstanceAir::Preset CurrentSet;
	CurrentSet.fill(*SourceGraphInstance->Instance);
	CurrentSet.apply(*DestGraphInstance->Instance);
}

void USubstanceUtility::ResetInputParameters(USubstanceGraphInstance* GraphInstance)
{
	if (!GraphInstance || !GraphInstance->Instance)
	{
		return;
	}

	Substance::Helpers::ResetToDefault(GraphInstance->Instance);
}

int SizeToPow2(ESubstanceTextureSize res)
{
	switch (res)
	{
	case ERL_16:
		return 4;
	case ERL_32:
		return 5;
	case ERL_64:
		return 6;
	case ERL_128:
		return 7;
	case ERL_256:
		return 8;
	case ERL_512:
		return 9;
	case ERL_1024:
		return 10;
	case ERL_2048:
		return 11;
	case ERL_4096:
		return 12;
	case ERL_8192:
		{
			//Check to make sure that we can support 8k textures
			if (GetDefault<USubstanceSettings>()->SubstanceEngine == ESubstanceEngineType::SET_GPU)
			{
				return 13;
			}
			else
			{
				UE_LOG(LogSubstanceCore, Warning, TEXT("Tried to assign texture size to 8k without using the GPU engine - Falling back to 2k!"));
				return 11;
			}
		}
	default:
		return 8;
	}
}

void USubstanceUtility::SetGraphInstanceOutputSize(USubstanceGraphInstance* GraphInstance, ESubstanceTextureSize Width, ESubstanceTextureSize Height)
{
	if (!GraphInstance || !GraphInstance->Instance || !GraphInstance->CanUpdate())
	{
		return;
	}

	TArray<int32> OutputSize;
	OutputSize.Add(SizeToPow2(Width));
	OutputSize.Add(SizeToPow2(Height));

	GraphInstance->SetInputInt(TEXT("$outputsize"), OutputSize);
}

void USubstanceUtility::SetGraphInstanceOutputSizeInt(USubstanceGraphInstance* GraphInstance, int32 Width, int32 Height)
{
	if (!GraphInstance || !GraphInstance->Instance || !GraphInstance->CanUpdate())
	{
		return;
	}

	Width = FMath::Log2(FMath::Pow(2.f, (int32)FMath::Log2(static_cast<float>(Width))));
	Height = FMath::Log2(FMath::Pow(2.f, (int32)FMath::Log2(static_cast<float>(Height))));

	TArray<int32> OutputSize;
	OutputSize.Add(Width);
	OutputSize.Add(Height);

	GraphInstance->SetInputInt(TEXT("$outputsize"), OutputSize);
}

void USubstanceUtility::ClearCache()
{
	Substance::Helpers::ClearCache();
}

USubstanceInstanceFactory* USubstanceUtility::CreateAggregateSubstanceFactory(
    USubstanceInstanceFactory* OutputFactory,
    int32 OutputFactoryGraphIndex,
    USubstanceInstanceFactory* InputFactory,
    int32 InputFactoryGraphIndex,
    const TArray<FSubstanceConnection>& Connections)
{
	//First we must resolve the GraphDesc's we plan to create an aggregate from
	const SubstanceAir::GraphDesc* OutputGraph = nullptr;
	const SubstanceAir::GraphDesc* InputGraph = nullptr;

	auto GetGraphAtIndex = [](USubstanceInstanceFactory * Factory, uint32 GraphIndex, const SubstanceAir::GraphDesc** OutGraph)
	{
		if (Factory && Factory->SubstancePackage && Factory->SubstancePackage->getGraphs().size() > GraphIndex && GraphIndex >= 0)
		{
			*OutGraph = &(Factory->SubstancePackage->getGraphs()[GraphIndex]);
		}
	};

	GetGraphAtIndex(OutputFactory, OutputFactoryGraphIndex, &OutputGraph);
	GetGraphAtIndex(InputFactory, InputFactoryGraphIndex, &InputGraph);

	if (!OutputGraph || !InputGraph)
	{
		UE_LOG(LogSubstanceCore, Warning, TEXT("Unable to resolve one or more GraphDesc's for CreateAggregateSubstanceFactory"));
		return nullptr;
	}

	SubstanceAir::ConnectionsOptions ConnectionOptions;

	//Now build the connection options list
	for (const auto& connection : Connections)
	{
		SubstanceAir::string OutputIdentifier(TCHAR_TO_UTF8(*connection.OutputIdentifier));
		SubstanceAir::string InputIdentifier(TCHAR_TO_UTF8(*connection.InputImageIdentifier));
		SubstanceAir::UInt outputUID = 0;
		SubstanceAir::UInt inputUID = 0;

		for (const auto& output : OutputGraph->mOutputs)
		{
			if (output.mIdentifier == OutputIdentifier)
			{
				outputUID = output.mUid;
				break;
			}
		}

		for (const auto& input : InputGraph->mInputs)
		{
			if (input->mIdentifier == InputIdentifier)
			{
				inputUID = input->mUid;
				break;
			}
		}

		ConnectionOptions.mConnections.push_back(std::make_pair(outputUID, inputUID));
	}

	auto package = SubstanceAir::make_shared<SubstanceAir::PackageStackDesc>(*OutputGraph, *InputGraph, ConnectionOptions);
	if (package->isValid() == false)
	{
		UE_LOG(LogSubstanceCore, Warning, TEXT("Creating AggregatePackage failed in CreateAggregateSubstanceFactory"));
		return nullptr;
	}

	const char* AggregateLabel = package->getGraphs()[0].mLabel.c_str();
	USubstanceInstanceFactory* AggregateFactory = NewObject<USubstanceInstanceFactory>(GetTransientPackage(), AggregateLabel, RF_Transient);
	AggregateFactory->Initialize(package);

	return AggregateFactory;
}

void USubstanceUtility::SyncRendering(USubstanceGraphInstance* GraphInstance)
{
	if (!GraphInstance || !GraphInstance->Instance || !GraphInstance->CanUpdate())
	{
		return;
	}

	Substance::Helpers::RenderSync(GraphInstance->Instance);
}

void USubstanceUtility::AsyncRendering(USubstanceGraphInstance* GraphInstance)
{
	if (!GraphInstance || !GraphInstance->Instance || !GraphInstance->CanUpdate())
	{
		return;
	}

	Substance::Helpers::RenderAsync(GraphInstance->Instance);
}


TArray<UMaterial*> USubstanceUtility::GetSubstanceIncludedMaterials()
{
	return Substance::Helpers::GetSubstanceIncludedMaterials();
}
