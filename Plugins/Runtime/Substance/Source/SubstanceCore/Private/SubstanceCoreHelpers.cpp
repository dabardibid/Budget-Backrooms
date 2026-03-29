// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceCoreHelpers.cpp

#include "SubstanceCoreHelpers.h"
#include "SubstanceCorePrivatePCH.h"
#include "SubstanceCoreClasses.h"
#include "SubstanceGraphInstance.h"
#include "SubstanceInstanceFactory.h"
#include "SubstanceCallbacks.h"
#include "SubstanceOutputData.h"
#include "SubstanceTexture2D.h"

#include "substance/framework/output.h"
#include "substance/framework/input.h"
#include "substance/framework/inputimage.h"
#include "substance/framework/renderer.h"
#include "substance/framework/channel.h"
#include "substance/framework/package.h"
#include "substance/framework/preset.h"
#include "substance/framework/typedefs.h"
#include <substance/texture.h>

#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialLayersFunctions.h"
#include "Materials/MaterialExpressionOneMinus.h"

#include "Modules/ModuleManager.h"
#include "AssetRegistryModule.h"

#include "RenderCore.h"
#include "RenderUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/ConfigCacheIni.h"
#include "Engine/Texture2D.h"
#include "IImageWrapperModule.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/IPluginManager.h"

#if WITH_EDITOR
#include "Editor.h"
#include <DesktopPlatformModule.h>
#include "Interfaces/IMainFrameModule.h"
#include "SubstanceCoreModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "TexAlignTools.h"
#include "EditorReimportHandler.h"
#include "EditorSupportDelegates.h"
#include "UnrealEdClasses.h"
#include "ScopedTransaction.h"
#include "ObjectTools.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistryModule.h"
#include "Engine/AssetManager.h"
#endif

#if PLATFORM_PS4
#include "SubstanceCorePS4Utils.h"
#endif

namespace local
{
TArray<UObject*> InstancesToDelete;
TArray<UObject*> FactoriesToDelete;
TArray<UObject*> TexturesToDelete;
}

namespace Substance
{
//Pointer to the callbacks that should be fired from the Substance thread when an Async render task has been completed
TSharedPtr<Substance::RenderCallbacks> gCallbacks(new Substance::RenderCallbacks());

//The global renderer that is the link to the Substance engine used for processing Substances
TSharedPtr<SubstanceAir::Renderer> GSubstanceRenderer(nullptr);

//The global default render options - This will be filled out according to the plugin settings
SubstanceAir::RenderOptions GSubstanceDefaultRenderOptions;

//Stores how many mip maps we should create as place holders until processing complete
static const int32 MinPlaceHolderMips = 4;

//Queue used to limit the amount of substance that are pushed per frame while loading
TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>> LoadingQueue;

//Queue used to limit substances being rebuilt that will be loaded before LoadingQueue;
TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>> PriorityLoadingQueue;

//Queue used by RenderAsync
TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>> AsyncQueue;

//Queue used by RenderAsync
TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>> CurrentRenderSet;

//Stores the current job ID of an Async render task
uint32 ASyncRunID = 0;

//Used to calculate the loading percentage
static uint32 GlobalInstancePendingCount = 0;
static uint32 GlobalInstanceCompletedCount = 0;

//Flag for if we want to clear the substance cache during tick
static bool GClearSubstanceCache = false;

//Typedef for the link between the framework image input instance and the graph instance it is currently linked to
typedef std::pair<SubstanceAir::InputInstanceImage*, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>> GraphImageInputPair;

//Stores all of the ImageInputs we need to assign to graph instances and reprocess
TArray<GraphImageInputPair> DelayedImageInputs;

namespace Helpers
{
RenderCallbacks* GetRenderCallbacks()
{
	return gCallbacks.Get();
}

void RenderPush(USubstanceGraphInstance* Instance)
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	if (true == reinterpret_cast<GraphInstanceData*>(Instance->Instance->mUserData)->bHasPendingImageInputRendering)
	{
		GSubstanceRenderer->push(*Instance->Instance);
		return;
	}
}

void RenderAsync(TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>>& Instances)
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	for (const auto& GraphItr : Instances)
	{
		RenderAsync(GraphItr);
	}
}

void RenderAsync(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance)
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	AsyncQueue.AddUnique(Instance);
	++GlobalInstancePendingCount;
}

void RenderSync(TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>>& Instances, bool ForceCache)
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	GlobalInstanceCompletedCount += Instances.Num();
	GlobalInstancePendingCount += Instances.Num();

	GSubstanceRenderer->setRenderCallbacks(nullptr);
	for (const auto& InstanceItr : Instances)
	{
		GSubstanceRenderer->push(*InstanceItr.get());
	}
	GSubstanceRenderer->run(SubstanceAir::Renderer::Run_Default);
	GSubstanceRenderer->setRenderCallbacks(gCallbacks.Get());

	UpdateTextures(Instances, ForceCache);
}

void RenderSync(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, bool ForceCache)
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	++GlobalInstanceCompletedCount;
	++GlobalInstancePendingCount;

	GSubstanceRenderer->setRenderCallbacks(nullptr);
	GSubstanceRenderer->push(*Instance.get());
	GSubstanceRenderer->run(SubstanceAir::Renderer::Run_Default);
	GSubstanceRenderer->setRenderCallbacks(gCallbacks.Get());

	UpdateTextures(Instance, ForceCache);
}

void PushDelayedRender(USubstanceGraphInstance* Instance)
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	LoadingQueue.AddUnique(Instance->Instance);

}

void PushDelayedRender(SubstanceAir::InputInstanceImage* ImgInput, USubstanceGraphInstance* Instance)
{
	LoadingQueue.AddUnique(Instance->Instance);
}

void PushDelayedImageInput(SubstanceAir::InputInstanceImage* ImgInput, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance)
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	DelayedImageInputs.AddUnique(std::make_pair(ImgInput, Instance));
}

void RemoveFromDelayedImageInputs(SubstanceAir::InputInstanceImage* ImgInput)
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	//Local reference to allow the lambda to properly capture the loading queue
	auto CurrentLoadingQueue = &LoadingQueue;

	DelayedImageInputs.RemoveAll([ImgInput, CurrentLoadingQueue](const GraphImageInputPair & CurElement)
	{
		if (CurElement.first == ImgInput)
		{
			CurrentLoadingQueue->Remove(CurElement.second);
			return true;
		}
		else
		{
			return false;
		}
	});
}

void SetDelayedImageInput()
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	for (const auto& InputItr : DelayedImageInputs)
	{
		SubstanceAir::InputInstanceImage* ImgInput = InputItr.first;
		SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance = InputItr.second;

		if (ImgInput && ImgInput->getImage() && ImgInput->getImage()->mUserData != 0)
		{
			UObject* CurrentSource = reinterpret_cast<UTexture2D*>(ImgInput->getImage()->mUserData);
			SetImageInput(ImgInput, CurrentSource, Instance, true, false);
			reinterpret_cast<GraphInstanceData*>(Instance->mUserData)->bHasPendingImageInputRendering = false;
		}
	}

	DelayedImageInputs.Empty();
}

void PerformDelayedRender()
{
	if (IsRunningDedicatedServer())
	{
		return;
	}

	SetDelayedImageInput();

	//Rendering the loading queue over time.
	if (LoadingQueue.Num())
	{
		for (const auto& InstanceItr : LoadingQueue)
		{
			RenderAsync(InstanceItr);
		}
		//RenderAsync(LoadingQueue);
		LoadingQueue.Empty();
	}
}

SIZE_T CalculateSubstanceImageBytes(uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint8 Format)
{
	//NOTE:: We are defaulting to use our implementation because there is a bug within UE4s CalcualteImageBytes.
	//In UE4 - If the Size passed is less than the Block Size - It is rounded to 0 and we do not get the proper size back!
	if (Format == PF_A1)
	{
		// The number of bytes needed to store all 1 bit pixels in a line is the width of the image divided by the number of bits in a byte
		uint32 BytesPerLine = SizeX / 8;
		// The number of actual bytes in a 1 bit image is the bytes per line of pixels times the number of lines
		return sizeof(uint8) * BytesPerLine * SizeY;
	}
	else if (SizeZ > 0)
	{
		return (float)((float)SizeX / (float)GPixelFormats[Format].BlockSizeX) *
		       (float)((float)SizeY / (float)GPixelFormats[Format].BlockSizeY) *
		       (float)((float)SizeZ / (float)GPixelFormats[Format].BlockSizeZ) *
		       GPixelFormats[Format].BlockBytes;
	}
	else
	{
		return (float)((float)SizeX / (float)GPixelFormats[Format].BlockSizeX) *
		       (float)((float)SizeY / (float)GPixelFormats[Format].BlockSizeY) *
		       GPixelFormats[Format].BlockBytes;
	}
}

void UpdateSubstanceOutput(UTexture2D* TextureOutput, const SubstanceTexture& ResultText)
{
	
	// grab the Result computed in the Substance Thread
	const SIZE_T Mipstart = (SIZE_T)ResultText.buffer;
	SIZE_T MipOffset = 0;

	//Prepare mip map data
	FTexture2DMipMap* MipMap = nullptr;

	// Casting to UTexture to use virtual table for accessing UTexture2D::GetRunningPlatformData - this method is not exported with ENGINE_API,
	// so linker fails with unresolved external.
//	FTexturePlatformData* Texture = *TextureOutput->GetRunningPlatformData();
	FTexturePlatformData* Texture = *Cast<UTexture>(TextureOutput)->GetRunningPlatformData();

	if (Texture == nullptr)
		Texture = new FTexturePlatformData();

	EPixelFormat PreviousFormat = Texture->PixelFormat;

	Texture->PixelFormat = Substance::Helpers::SubstanceToUe4Format((SubstancePixelFormat)ResultText.pixelFormat, (SubstanceChannelsOrder)ResultText.channelsOrder);

	const bool bBlockSizeChanged = (PreviousFormat != Texture->PixelFormat)
									&& (GPixelFormats[PreviousFormat].BlockSizeX != GPixelFormats[Texture->PixelFormat].BlockSizeX
									|| GPixelFormats[PreviousFormat].BlockSizeY != GPixelFormats[Texture->PixelFormat].BlockSizeY);

	 
	//Create as much mip as necessary
	if (Texture->Mips.Num() != ResultText.mipmapCount 
		|| ResultText.level0Width != Texture->SizeX 
		|| ResultText.level0Height != Texture->SizeY
		|| bBlockSizeChanged
#if WITH_EDITORONLY_DATA
		|| TextureOutput->MipGenSettings == TextureMipGenSettings::TMGS_LeaveExistingMips
#endif
		)
	{
		TextureOutput->ReleaseResource();

		Texture->Mips.Empty();

		int32 MipSizeX = Texture->SizeX = ResultText.level0Width;
		int32 MipSizeY = Texture->SizeY = ResultText.level0Height;

		for (int32 IdxMip = 0; IdxMip < ResultText.mipmapCount; ++IdxMip)
		{
			MipMap = new FTexture2DMipMap();
			Texture->Mips.Add(MipMap);
			MipMap->SizeX = MipSizeX;
			MipMap->SizeY = MipSizeY;

			//Compute the next mip size
			MipSizeX = FMath::Max(MipSizeX >> 1, 1);
			MipSizeY = FMath::Max(MipSizeY >> 1, 1);

			//Not smaller than the "block size"
			MipSizeX = FMath::Max((int32)GPixelFormats[Texture->PixelFormat].BlockSizeX, MipSizeX);
			MipSizeY = FMath::Max((int32)GPixelFormats[Texture->PixelFormat].BlockSizeY, MipSizeY);
		}
	}

	//NOTE:: The last two mips (1x1 and 2x2) cause issuses with the size - These should never really be needed
	//Fill up the mips
	for (int32 IdxMip = 0; IdxMip < ResultText.mipmapCount; ++IdxMip)
	{
		MipMap = &Texture->Mips[IdxMip];

		//Get the size of the mip's content
		const SIZE_T ImageSize = Substance::Helpers::CalculateSubstanceImageBytes(
		                             MipMap->SizeX,
		                             MipMap->SizeY,
		                             0,
		                             Texture->PixelFormat);
		check(0 != ImageSize);

#if PLATFORM_PS4
		TileMipForPS4((void*)(Mipstart + MipOffset), MipMap, Texture->PixelFormat);
		MipOffset += ImageSize;
#else
		void* TheMipDataPtr = nullptr;

		//Copy the data
		//some flags may not allow mips to be loaded into memory and will crash on lock for read/write
		if (MipMap->BulkData.GetBulkDataSize() != ImageSize 
			|| MipMap->BulkData.GetBulkDataFlags() & EBulkDataFlags::BULKDATA_PayloadInSeperateFile 
			|| MipMap->BulkData.GetBulkDataFlags() & EBulkDataFlags::BULKDATA_SingleUse)
		{
			MipMap->BulkData = FByteBulkData();
			MipMap->BulkData.Lock(LOCK_READ_WRITE);
			TheMipDataPtr = MipMap->BulkData.Realloc(ImageSize);
		}
		else
		{
			TheMipDataPtr = MipMap->BulkData.Lock(LOCK_READ_WRITE);
		}

		if (Texture->PixelFormat == PF_B8G8R8A8)
		{
			uint8* SrcPixels = (uint8*)(Mipstart + MipOffset);
			uint8* DstPixels = (uint8*)TheMipDataPtr;

			//Swizzle channel order
			int indices[] =
			{
				ResultText.channelsOrder & 0x3,
				(ResultText.channelsOrder >> 2) & 0x3,
				(ResultText.channelsOrder >> 4) & 0x3,
				(ResultText.channelsOrder >> 6) & 0x3,
			};

			for (SIZE_T Idx = 0; Idx < ImageSize; Idx += 4)
			{
				//Substance outputs rgba8, convert to bgra8
				// r, g, b, a = b, g, r, a
				DstPixels[Idx + 0] = SrcPixels[Idx + indices[2]];
				DstPixels[Idx + 1] = SrcPixels[Idx + indices[1]];
				DstPixels[Idx + 2] = SrcPixels[Idx + indices[0]];
				DstPixels[Idx + 3] = SrcPixels[Idx + indices[3]];
			}
		}
		else
		{
			FMemory::Memcpy(TheMipDataPtr, (void*)(Mipstart + MipOffset), ImageSize);
		}

		MipOffset += ImageSize;
		MipMap->BulkData.ClearBulkDataFlags(BULKDATA_SingleUse);
		MipMap->BulkData.Unlock();
#endif
	}

}

void UpdateSubstanceOutputSource(UTexture2D* TextureOutput, const SubstanceTexture& ResultText)
{
	//Store the platform data and its mipmap data structure
//	FTexturePlatformData* TexturePlatformData = *TextureOutput->GetRunningPlatformData();
	FTexturePlatformData* TexturePlatformData = *Cast<UTexture>(TextureOutput)->GetRunningPlatformData();

#if WITH_EDITORONLY_DATA
	//Determine which Source Texture Format to use
	SubstancePixelFormat PixelFormat = (SubstancePixelFormat)ResultText.pixelFormat;
	ETextureSourceFormat SourceDataFormat = SubstanceToUE4SourceFormat(PixelFormat);

	if (SourceDataFormat == TSF_Invalid)
		return;
	
	int32 TopMipSizeX = TexturePlatformData->SizeX = ResultText.level0Width;
	int32 TopMipSizeY = TexturePlatformData->SizeY = ResultText.level0Height;

	//#NOTE:: This is a slow operation as this updates DDC, runs through compressor and also updates textureRHI
	TextureOutput->Source.Init(TopMipSizeX, TopMipSizeY, 1, ResultText.mipmapCount, SourceDataFormat, (uint8*)ResultText.buffer);

	//Flag for save
	TextureOutput->MarkPackageDirty();
#endif
}

void UpdateTexture(const SubstanceTexture& result, SubstanceAir::OutputInstance* Output, bool bCacheResults /*= true*/)
{
	if (Output->mUserData == 0)
		return;

	USubstanceOutputData* OutputData = reinterpret_cast<USubstanceOutputData*>(Output->mUserData);
	//Local reference of the texture we will be updating
	UTexture2D* TextureOutput = Cast<UTexture2D>(OutputData->GetData());

	//If the texture is invalid, we can't update it so we are done here
	if (!TextureOutput)
		return;

	TextureOutput->ConditionalPostLoad();

	//Updates the buffer and tells the RHI to push the new data to the GPU
	if (bCacheResults)
	{
		Helpers::UpdateSubstanceOutputSource(TextureOutput, result);
	}
	else
	{
#if WITH_EDITORONLY_DATA
		TextureOutput->ClearAllCachedCookedPlatformData();

#endif
		Helpers::UpdateSubstanceOutput(TextureOutput, result);
#if WITH_EDITORONLY_DATA
		TextureOutput->MarkPackageDirty();
#endif

	}

	TextureOutput->UpdateResource();
}

void OnWorldInitialized()
{
	GlobalInstanceCompletedCount = 0;
	GlobalInstancePendingCount = 0;

	PerformDelayedRender();
}

void OnLevelAdded()
{
	PerformDelayedRender();
}

SubstanceAir::OutputInstance::Result getNewestOutputResult(SubstanceAir::OutputInstance* output)
{
	SubstanceAir::OutputInstance::Result result = nullptr;

	if (output != nullptr)
	{
		for (SubstanceAir::OutputInstance::Result nextResult = output->grabResult(); nextResult != nullptr;
			 nextResult = output->grabResult())
		{
			result = std::move(nextResult);
		}
	}

	return result;
}

void StartPIE()
{
#if WITH_EDITOR
	GSubstanceRenderer->cancelAll();

	//Update outputs
	bool bUpdatedOutput = false;

	for (const auto& OutputItr : Substance::Helpers::GetRenderCallbacks()->getComputedOutputs(false))
	{
		if (OutputItr->mDesc.isImage())
		{
			SubstanceAir::OutputInstance::Result result = Substance::Helpers::getNewestOutputResult(OutputItr);
			SubstanceAir::RenderResultImage* resultImage = static_cast<SubstanceAir::RenderResultImage*>(result.get());

			if (resultImage)
			{
				bool isEditor = false;
#if WITH_EDITOR
				FSubstanceCoreModule& SubstanceModule = FModuleManager::LoadModuleChecked<FSubstanceCoreModule>(TEXT("SubstanceCore"));
				const bool PIE = SubstanceModule.isPie();
				if (!PIE)
					isEditor = true;
#endif
				UpdateTexture(resultImage->getTexture(), OutputItr, isEditor);
				bUpdatedOutput = true;
			}
		}
		else
		{
			USubstanceOutputData* OutputData = reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData);
			if (OutputData)
			{
				SetMaterialExpression(OutputItr, OutputData->ParentInstance->ConstantCreatedMaterial, OutputData->ParamInfo);
			}
		}
	}

#endif //WITH_EDITOR
}

void EndPIE()
{
#if WITH_EDITOR


#endif //WITH_EDITOR
}

void Tick()
{
#if WITH_EDITOR
	FSubstanceCoreModule& SubstanceModule = FModuleManager::LoadModuleChecked<FSubstanceCoreModule>(TEXT("SubstanceCore"));
	const bool PIE = SubstanceModule.isPie();
#endif

	//NOTE:: No longer throttling output. This can cause performance issues when many substances are being updated per frame!
	// If we are to add this back, allow more then one output to be updated per frame and give the option to the user capped between 1 and MAX
	// outputs that can be updated each frame.
	TArray<SubstanceAir::OutputInstance*> Outputs = Substance::Helpers::GetRenderCallbacks()->getComputedOutputs(false);
	bool bUpdatedOutput = false;

	for (const auto& OutputItr : Outputs)
	{
		if (OutputItr->mDesc.isImage())
		{
			SubstanceAir::OutputInstance::Result result = Substance::Helpers::getNewestOutputResult(OutputItr);
			SubstanceAir::RenderResultImage* resultImage = static_cast<SubstanceAir::RenderResultImage*>(result.get());

			if (resultImage)
			{
				UpdateTexture(resultImage->getTexture(), OutputItr, false);
#if WITH_EDITOR
				if (OutputItr->mUserData != 0)
				{
					auto texture = reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData)->GetData();
                    if(texture)
                    {
                        texture->PostEditChange();
                    }
				}
#endif
				bUpdatedOutput = true;
			}
		}
		else
		{
			USubstanceOutputData* OutputData = reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData);
			if (OutputData)
			{
#if WITH_EDITOR			
				SetMaterialExpression(OutputItr, OutputData->ParentInstance->ConstantCreatedMaterial, OutputData->ParamInfo);
#else
				SetMaterialExpression(OutputItr, OutputData->ParentInstance->DynamicCreatedMaterial, OutputData->ParamInfo);
#endif
			}
		}
	}

	//NOTE:: This is needed to show updates in the viewport live as changes are made to input values!
#if WITH_EDITOR
	if (bUpdatedOutput)
	{
		//Refresh the editor view ports
		FEditorSupportDelegates::RedrawAllViewports.Broadcast();
	}
#endif

	//Check to see if there is a job pending and if so, is the job complete?
	bool BatchFired = false;
	if (!GSubstanceRenderer->isPending(ASyncRunID))
	{
		ASyncRunID = 0;
		GlobalInstanceCompletedCount += CurrentRenderSet.Num();

		//Has the cache been ordered to clear?
		if (GClearSubstanceCache)
		{
			GClearSubstanceCache = false;

			SubstanceAir::RenderOptions clearOptions;
			clearOptions.mMemoryBudget = SUBSTANCE_MEMORYBUDGET_FORCEFLUSH;

			GSubstanceRenderer->setOptions(clearOptions);
			GSubstanceRenderer->run();
			GSubstanceRenderer->setOptions(GSubstanceDefaultRenderOptions);
		}

		//Limit the amount of substances that can be pushed per frame based on user option
		int ClampedUpdateCound = FMath::Clamp(GetDefault<USubstanceSettings>()->MaxAsyncSubstancesRenderedPerFrame, 0, AsyncQueue.Num());
		CurrentRenderSet.Empty(ClampedUpdateCound);
		for (int i = 0; i < ClampedUpdateCound; ++i)
		{
			std::shared_ptr<SubstanceAir::GraphInstance> CurrentGraph = AsyncQueue.Pop();
			GSubstanceRenderer->push(*CurrentGraph);
			BatchFired = true;
			CurrentRenderSet.AddUnique(CurrentGraph);
		}
	}

	//Fire off the substance render thread when a set has been pushed
	if (BatchFired)
	{
		ASyncRunID = GSubstanceRenderer->run(
		                 SubstanceAir::Renderer::Run_Asynchronous |
		                 SubstanceAir::Renderer::Run_Replace |
		                 SubstanceAir::Renderer::Run_First |
		                 SubstanceAir::Renderer::Run_PreserveRun);
	}

	//Clean up objects and trigger garbage collection
	Substance::Helpers::PerformDelayedDeletion();
}

bool isSupportedByDefaultShader(SubstanceAir::OutputInstance* Output)
{
	check(Output);

	//Create output if it does match to a slot of the default material
	switch (Output->mDesc.defaultChannelUse())
	{
	case SubstanceAir::Channel_BaseColor:
	case SubstanceAir::Channel_Metallic:
	case SubstanceAir::Channel_SpecularColor:
	case SubstanceAir::Channel_Specular:
	case SubstanceAir::Channel_Roughness:
	case SubstanceAir::Channel_Emissive:
	case SubstanceAir::Channel_Normal:
	case SubstanceAir::Channel_Mask:
	case SubstanceAir::Channel_Opacity:
	case SubstanceAir::Channel_Refraction:
	case SubstanceAir::Channel_AmbientOcclusion:
		return true;
	default:
		return false;
	}
}

void EnableTexture(SubstanceAir::OutputInstance* Output, USubstanceGraphInstance* Graph, const OutputTextureSettings* TextureSettings, bool IsTransient)
{
	FString TextureName;
	FString PackageName;
	UObject* Outer = nullptr;

	if (!TextureSettings)
	{
		GetSuitableName(Output, TextureName, PackageName, Graph);
	}
	else
	{
		//Get the readable UAsset name from given the path
		TextureSettings->PackageName.Split(TEXT("/"), nullptr, &(TextureName), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		PackageName = TextureSettings->PackageName;
	}

	if (IsTransient)
	{
		Outer = Graph->GetOuter();
		PackageName = TEXT("/") + PackageName;
	}

#if WITH_EDITOR
	FSubstanceCoreModule& SubstanceModule = FModuleManager::LoadModuleChecked<FSubstanceCoreModule>(TEXT("SubstanceCore"));
	const bool isTransient = SubstanceModule.isPie();
#else
	const bool isTransient = true;
#endif

	UObject* TextureParent = CreatePackage(*PackageName);
	CreateSubstanceTexture2D(Output, isTransient, TextureName, TextureParent, Graph);

	//Restore settings of created texture is available
	if (TextureSettings)
	{
		Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(Output->mUserData)->GetData())->SRGB = TextureSettings->sRGB_Enabled;
	}
}

void CreateTextures(USubstanceGraphInstance* Graph, bool ForceCreate, bool IsTransient)
{
	for (const auto& Output : Graph->Instance->getOutputs())
	{
		if (Output->mEnabled && Output->mDesc.isImage())
		{
			EnableTexture(Output, Graph, nullptr, IsTransient);
		}
		else if (Output->mDesc.isNumerical())
		{
			USubstanceOutputData* OutputLinkData = nullptr;
			if (!ForceCreate)
			{
				// Reuse existing USubstanceOutputData object
				USubstanceOutputData** pOutputLinkData = Graph->OutputInstances.Find(Output->mDesc.mUid);
				if (pOutputLinkData && *pOutputLinkData != nullptr)
				{
					OutputLinkData = *pOutputLinkData;
					// Just in case, let's clean up the existing object from old data
					OutputLinkData->ConnectedObject = nullptr;
					OutputLinkData->ParamInfo = FMaterialParameterInfo();
				}
			}
			if (OutputLinkData == nullptr)
			{
				OutputLinkData = NewObject<USubstanceOutputData>(Graph, FName(), RF_Public);
				Graph->OutputInstances.Add(Output->mDesc.mUid, OutputLinkData);
			}
			OutputLinkData->ParentInstance = Graph;
			OutputLinkData->CacheGuid = FGuid::NewGuid();
			Output->mUserData = (size_t)OutputLinkData;
			Output->mEnabled = true;

		}
	}
	Substance::Helpers::CreatePlaceHolderMips(Graph->Instance);
}

bool UpdateTextures(TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>>& Instances, bool InEditor)
{
	bool GotSomething = false;

	for (const auto& GraphInstItr : Instances)
	{
		GotSomething = UpdateTextures(GraphInstItr, InEditor) ? true : GotSomething;
	}

	return GotSomething;
}

bool UpdateTextures(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, bool InEditor)
{
	//Iterate on all Outputs
	bool GotSomething = false;

	for (const auto& OutputItr : Instance->getOutputs())
	{
		//Make sure the user data is valid
		if (OutputItr->mUserData == 0)
		{
			continue;
		}

		if (OutputItr->mDesc.isImage())
		{
			SubstanceAir::OutputInstance::Result result = Substance::Helpers::getNewestOutputResult(OutputItr);
			SubstanceAir::RenderResultImage* resultImage = static_cast<SubstanceAir::RenderResultImage*>(result.get());

			if (resultImage)
			{
				TWeakObjectPtr<USubstanceGraphInstance> UEGraphInstance = reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData)->ParentInstance;
				UEGraphInstance->MarkPackageDirty();
				UpdateTexture(resultImage->getTexture(), OutputItr, InEditor);
				GotSomething = true;
			}
		}
		else
		{
			USubstanceOutputData* OutputData = reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData);
			if (OutputData)
			{
#if WITH_EDITOR			
				SetMaterialExpression(OutputItr, OutputData->ParentInstance->ConstantCreatedMaterial, OutputData->ParamInfo);
#else
				SetMaterialExpression(OutputItr, OutputData->ParentInstance->DynamicCreatedMaterial, OutputData->ParamInfo);
#endif
			}
		}
	}

	return GotSomething;
}

SUBSTANCECORE_API SubstanceAir::OutputInstance* GetSubstanceOutputByID(USubstanceGraphInstance* graph, uint32 UID)
{
	return graph->Instance->findOutput(UID);
}

void CreateSubstanceTexture2D(SubstanceAir::OutputInstance* Output, bool Transient, FString Name, UObject* InOuter, USubstanceGraphInstance* ParentInst)
{
	check(Output);
	check(InOuter);

	UTexture2D* Texture = nullptr;
	Texture = NewObject<UTexture2D>(InOuter, FName(*Name), Transient ? RF_NoFlags : RF_Public | RF_Standalone);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().AssetCreated(Texture);

	Texture->MarkPackageDirty();
	(*Cast<UTexture>(Texture)->GetRunningPlatformData()) = new FTexturePlatformData();
	FTexturePlatformData* TexturePlatformData = *Cast<UTexture>(Texture)->GetRunningPlatformData();
	Texture->AddressX = TextureAddress::TA_Wrap;
	Texture->AddressY = TextureAddress::TA_Wrap;

	// Reuse existing USubstanceOutputData object
	USubstanceOutputData* OutputLinkData = nullptr;
	USubstanceOutputData** pOutputLinkData = ParentInst->OutputInstances.Find(Output->mDesc.mUid);
	if (pOutputLinkData && *pOutputLinkData != nullptr)
	{
		OutputLinkData = *pOutputLinkData;
		// Just in case, let's clean up the existing object from old data
		OutputLinkData->ParamInfo = FMaterialParameterInfo();
	}
	else
	{
		OutputLinkData = NewObject<USubstanceOutputData>(ParentInst,FName(), RF_Public);
		ParentInst->OutputInstances.Add(Output->mDesc.mUid, OutputLinkData);
	}

	OutputLinkData->ParentInstance = ParentInst;
	OutputLinkData->SetData(Texture);
	OutputLinkData->CacheGuid = FGuid::NewGuid();

	Output->mUserData = (size_t)OutputLinkData;

	const SubstanceAir::OutputDesc* Desc = &Output->mDesc;
	const SubstanceAir::ChannelUse OutputChannel = Desc->defaultChannelUse();

	if (OutputChannel == SubstanceAir::Channel_Normal)
	{
		Texture->SRGB = false;
		Texture->CompressionSettings = TC_Normalmap;
	}
	else if (OutputChannel == SubstanceAir::Channel_BaseColor
		|| OutputChannel == SubstanceAir::Channel_Diffuse
		|| OutputChannel == SubstanceAir::Channel_Specular
		|| OutputChannel == SubstanceAir::Channel_Emissive)
	{
		Texture->SRGB = true;
	}
	else if (OutputChannel == SubstanceAir::Channel_Roughness
		|| OutputChannel == SubstanceAir::Channel_Metallic
		|| OutputChannel == SubstanceAir::Channel_SpecularLevel
		|| OutputChannel == SubstanceAir::Channel_Glossiness
		|| OutputChannel == SubstanceAir::Channel_AmbientOcclusion
		|| OutputChannel == SubstanceAir::Channel_Opacity
		|| OutputChannel == SubstanceAir::Channel_Displacement
		|| OutputChannel == SubstanceAir::Channel_Height)
	{
		Texture->SRGB = false;
		Texture->CompressionSettings = TC_Grayscale;
	}

	//Set the initial format. This will be overwritten on output computed.
	TexturePlatformData->PixelFormat = Substance::Helpers::SubstanceToUe4Format((SubstancePixelFormat)Output->mDesc.mFormat, Substance_ChanOrder_RGBA);

#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TextureMipGenSettings::TMGS_LeaveExistingMips;
#endif

	//Unsupported format
	if (PF_Unknown == Texture->GetPixelFormat())
	{
		Texture->ClearFlags(RF_Standalone);
		return;
	}

	switch (Texture->GetPixelFormat())
	{
	case PF_G8:
		Texture->SRGB = false;
		check(Output->mDesc.defaultChannelUse() != SubstanceAir::Channel_Normal);
		break;
	case PF_G16:
		Texture->SRGB = false;
		break;
#if WITH_EDITOR
	case PF_DXT1:
		Texture->CompressionNoAlpha = true;
		break;
	case PF_ATC_RGB:
		Texture->CompressionNoAlpha = true;
		break;
	case PF_ETC1:
		Texture->CompressionNoAlpha = true;
		break;
	case PF_ETC2_RGB:
		Texture->CompressionNoAlpha = true;
		break;
#endif //WITH_EDITOR
	}

	Texture->PlatformData->SizeX = 0;
	Texture->PlatformData->SizeY = 0;

#if WITH_EDITORONLY_DATA
	Texture->UpdateResource();
	Texture->PostEditChange();
#endif

	Output->mEnabled = true;
	Output->flagAsDirty();
}

void CreatePlaceHolderMips(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance)
{
	check(Instance);
	check(Instance->getOutputs().size());

	//For each enabled output, create a placeholder mipmap pyramid
	//with the following format & size : PF_B8G8R8A8 16x16
	for (const auto& OutputItr : Instance->getOutputs())
	{
		if (false == OutputItr->mEnabled || OutputItr->mUserData == 0)
		{
			continue;
		}

		UTexture2D* OutputTexture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData)->GetData());

		if (OutputTexture == nullptr)
		{
			continue;
		}

		FTexturePlatformData* Texture = OutputTexture->PlatformData;
		FTexture2DMipMap* MipMap = nullptr;

		if (Texture == nullptr)
		{
			Texture = new FTexturePlatformData();
		}

		Texture->PixelFormat = PF_B8G8R8A8;

		Texture->Mips.Empty();

		int32 MipSizeX = Texture->SizeX = FMath::Pow(2.f, MinPlaceHolderMips);
		int32 MipSizeY = Texture->SizeY = FMath::Pow(2.f, MinPlaceHolderMips);
		SIZE_T MipOffset = 0;

		for (int32 IdxMip = 0; IdxMip < MinPlaceHolderMips; ++IdxMip)
		{
			MipMap = new FTexture2DMipMap();
			Texture->Mips.Add(MipMap);
			MipMap->SizeX = MipSizeX;
			MipMap->SizeY = MipSizeY;

			//Compute the next mip size
			MipSizeX = FMath::Max(MipSizeX >> 1, 1);
			MipSizeY = FMath::Max(MipSizeY >> 1, 1);

			//Not smaller than the "block size"
			MipSizeX = FMath::Max((int32)GPixelFormats[Texture->PixelFormat].BlockSizeX, MipSizeX);
			MipSizeY = FMath::Max((int32)GPixelFormats[Texture->PixelFormat].BlockSizeY, MipSizeY);
		}

		//Fill up the mips
		for (int32 IdxMip = 0; IdxMip < Texture->Mips.Num(); ++IdxMip)
		{
			MipMap = &Texture->Mips[IdxMip];

			//Get the size of the mip's content
			const SIZE_T ImageSize = CalculateSubstanceImageBytes(
			                             MipMap->SizeX,
			                             MipMap->SizeY,
			                             0,
			                             Texture->PixelFormat);
			check(0 != ImageSize);

			//Copy the data
			MipMap->BulkData = FByteBulkData();
			MipMap->BulkData.Lock(LOCK_READ_WRITE);
			void* TheMipDataPtr = MipMap->BulkData.Realloc(ImageSize);

			uint8* Pixels = (uint8*)TheMipDataPtr;

			for (SIZE_T Idx = 0; Idx < ImageSize; Idx += 4)
			{
				Pixels[0] = 128;
				Pixels[1] = 128;
				Pixels[2] = 128;
				Pixels[3] = 0;
				Pixels += 4;
			}

			MipOffset += ImageSize;
			MipMap->BulkData.ClearBulkDataFlags(BULKDATA_SingleUse);
			MipMap->BulkData.Unlock();
		}

		OutputTexture->UpdateResource();
	}
}

SubstancePixelFormat UE4FormatToSubstance(EPixelFormat Fmt)
{
	switch (Fmt)
	{
	case PF_DXT1:
		return Substance_PF_DXT1;
	case PF_DXT3:
		return Substance_PF_DXT3;
	case PF_DXT5:
		return Substance_PF_DXT5;
	case PF_BC4:
		return Substance_PF_BC4;
	case PF_BC5:
		return Substance_PF_BC5;
	case PF_R8G8B8A8:
	case PF_B8G8R8A8:
	case PF_A8R8G8B8:
		return Substance_PF_RGBA;
	case PF_G8:
		return Substance_PF_L;
	case PF_G16:
		return SubstancePixelFormat(Substance_PF_L | Substance_PF_16b);
	default:
		return Substance_PF_RGBA;
	}
}

EPixelFormat SubstanceToUe4Format(const SubstancePixelFormat Format, const SubstanceChannelsOrder ChanOrder)
{
	//We are stripping sRGB (linear) here so that we can use a switch to check values regardless of if sRGB is toggled on or not.
	SubstancePixelFormat InFmt = SubstancePixelFormat(Format & ~Substance_PF_sRGB);

	EPixelFormat OutFormat = PF_Unknown;

	switch ((unsigned int)InFmt)
	{
	case Substance_PF_L | Substance_PF_16b:
		OutFormat = PF_G16;
		break;

	case Substance_PF_L | Substance_PF_FP:
		OutFormat = PF_R32_FLOAT;
		break;

	case Substance_PF_L | Substance_PF_16F:
		OutFormat = PF_R16F;
		break;

	case Substance_PF_L:
		OutFormat = PF_G8;
		break;

	case Substance_PF_DXT1:
		OutFormat = PF_DXT1;
		break;

	case Substance_PF_DXT3:
		OutFormat = PF_DXT3;
		break;

	case Substance_PF_DXT5:
		OutFormat = PF_DXT5;
		break;

	case Substance_PF_BC5:
		OutFormat = PF_BC5;
		break;

	case Substance_PF_32F:
		OutFormat = PF_A32B32G32R32F;
		break;

	case Substance_PF_16F:
		OutFormat = PF_FloatRGBA;
		break;

	case Substance_PF_RGBA | Substance_PF_16b:
	case Substance_PF_RGB | Substance_PF_16b:
	case Substance_PF_RGBx | Substance_PF_16b:
		OutFormat = PF_R16G16B16A16_UINT;
		break;

	case Substance_PF_RGBA:
	case Substance_PF_RGBx:
	case Substance_PF_RGB:
		{
			if (ChanOrder == Substance_ChanOrder_ARGB)
			{
				OutFormat = PF_A8R8G8B8;
			}
			else if (ChanOrder == Substance_ChanOrder_RGBA)
			{
				OutFormat = PF_R8G8B8A8;
			}
			else
			{
				OutFormat = PF_B8G8R8A8; //We special case BGRA to read swizzling later (ABGR etc)
			}
		}
		break;

	//All other formats are replaced by one of the previous ones (DXT5 mostly)
	default:
		OutFormat = PF_Unknown;
		break;
	}

	return OutFormat;
}

void OverwriteSubstancePixelFormatForSourceImage(SubstanceAir::OutputInstance* Output)
{
	const SubstanceAir::OutputDesc* Desc = &Output->mDesc;
	SubstanceAir::OutputFormat FormatOverride;
	FormatOverride.mipmapLevelsCount = SubstanceAir::OutputFormat::Constant::MipmapFullPyramid;
	FormatOverride.format = Desc->mFormat;
	int32 NewFormat = Desc->mFormat;

	//#NOTE:: Available Raw Image Formats
	//G8, BGRA8, BGRE8, RGBA16, RGBA16F
	//Currently there is an issue initializing with any format other than RAW to BGRE8
	NewFormat = Substance_PF_RAW;

	//Swizzle the channel order to line up with the needed BGRA8 (UE4 deprecated RGBA for raw image formats)
    const auto platformName = UGameplayStatics::GetPlatformName();
    if (platformName != "Mac" && platformName != "Linux")
    {
    	FormatOverride.perComponent[0].shuffleIndex = 2;
    	FormatOverride.perComponent[1].shuffleIndex = 1;
    	FormatOverride.perComponent[2].shuffleIndex = 0;
    	FormatOverride.perComponent[3].shuffleIndex = 3;
    }

	FormatOverride.format = NewFormat;
	Output->overrideFormat(FormatOverride);
}

void OverwriteSubstancePixelFormatForRuntimeCompression(SubstanceAir::OutputInstance* Output)
{
	const SubstanceAir::OutputDesc* Desc = &Output->mDesc;
	SubstanceAir::OutputFormat FormatOverride;
	FormatOverride.mipmapLevelsCount = SubstanceAir::OutputFormat::Constant::MipmapFullPyramid;
	FormatOverride.format = Desc->mFormat;
	int32 NewFormat = Desc->mFormat;

	UTexture2D* OutputTexture = nullptr;
	if (Output->mUserData)
	{
		OutputTexture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(Output->mUserData)->GetData());
	}

	if(OutputTexture != nullptr)
	{ 
		NewFormat = UE4FormatToSubstance(OutputTexture->GetPixelFormat());
	}
	else
	{
		if (Output->mDesc.defaultChannelUse() == SubstanceAir::Channel_Normal)
		{
			NewFormat = Substance_PF_BC5;
		}

		switch (NewFormat)
		{
			//G8 - 8 bit Single Channel
		case Substance_PF_L | Substance_PF_16b:
		case Substance_PF_L | Substance_PF_16F:
		case Substance_PF_L:
			NewFormat = Substance_PF_L;
			break;

			//Normal map stays BC5
		case Substance_PF_BC5:
			NewFormat = Substance_PF_BC5;
			break;

			//16 bit 4 channel half float
		case Substance_PF_16F:
		case Substance_PF_RGBA | Substance_PF_16b:
		case Substance_PF_RGB | Substance_PF_16b:
		case Substance_PF_RGBx | Substance_PF_16b:
			NewFormat = Substance_PF_DXT1;
			break;

			//defaults to 8bit 4 channels (swizzled)
		default:
		{
			NewFormat = Substance_PF_DXT1;
			break;
		}
		}
	}

	FormatOverride.format = NewFormat;
	Output->overrideFormat(FormatOverride);
}


ETextureSourceFormat SubstanceToUE4SourceFormat(const SubstancePixelFormat Format)
{
	switch ((unsigned int)Format)
	{
	case Substance_PF_L:
		return TSF_G8;

	case Substance_PF_16F:
	case Substance_PF_L | Substance_PF_16F:
		return TSF_RGBA16F;

		//Prevent crash because there is no one to one format conversion between R8G8 to the Source formats
	case Substance_PF_RGBA:
	case Substance_PF_RGBx:
	case Substance_PF_RGB:
		return TSF_BGRA8;

	case Substance_PF_L | Substance_PF_16b:
	case Substance_PF_RGBA | Substance_PF_16b:
	case Substance_PF_RGB | Substance_PF_16b:
	case Substance_PF_RGBx | Substance_PF_16b:
		return TSF_RGBA16;
	}

	return TSF_Invalid;
}

bool AreInputValuesEqual(SubstanceAir::InputInstanceBase& A, SubstanceAir::InputInstanceBase& B)
{
	using namespace SubstanceAir;
	SubstanceAir::InputInstanceBase* InstanceA = &A;
	SubstanceAir::InputInstanceBase* InstanceB = &B;

	if (!InstanceA || !InstanceB)
	{
		check(0);
		return false;
	}

	//Don't bother comparing values of inputs that don't relate to the same Input
	if ((InstanceA->mDesc.mUid != InstanceB->mDesc.mUid) || (InstanceA->mDesc.mType != InstanceB->mDesc.mType))
	{
		return false;
	}

	switch ((SubstanceIOType)InstanceA->mDesc.mType)
	{
	case Substance_IOType_Float:
		{
			InputInstanceNumerical<float>* InputA = (InputInstanceNumerical<float>*)InstanceA;
			InputInstanceNumerical<float>* InputB = (InputInstanceNumerical<float>*)InstanceB;

			return FMath::IsNearlyEqual(InputA->getValue(), InputB->getValue(), (float)DELTA);
		}
		break;
	case Substance_IOType_Float2:
		{
			InputInstanceNumerical<SubstanceAir::Vec2Float>* InputA = (InputInstanceNumerical<SubstanceAir::Vec2Float>*)InstanceA;
			InputInstanceNumerical<SubstanceAir::Vec2Float>* InputB = (InputInstanceNumerical<SubstanceAir::Vec2Float>*)InstanceB;

			return
			    FMath::IsNearlyEqual(InputA->getValue().x, InputB->getValue().x, (float)DELTA) &&
			    FMath::IsNearlyEqual(InputA->getValue().y, InputB->getValue().y, (float)DELTA);
		}
		break;
	case Substance_IOType_Float3:
		{
			InputInstanceNumerical<SubstanceAir::Vec3Float>* InputA = (InputInstanceNumerical<SubstanceAir::Vec3Float>*)InstanceA;
			InputInstanceNumerical<SubstanceAir::Vec3Float>* InputB = (InputInstanceNumerical<SubstanceAir::Vec3Float>*)InstanceB;

			return
			    FMath::IsNearlyEqual(InputA->getValue().x, InputB->getValue().x, (float)DELTA) &&
			    FMath::IsNearlyEqual(InputA->getValue().y, InputB->getValue().y, (float)DELTA) &&
			    FMath::IsNearlyEqual(InputA->getValue().z, InputB->getValue().z, (float)DELTA);
		}
		break;
	case Substance_IOType_Float4:
		{
			InputInstanceNumerical<SubstanceAir::Vec4Float>* InputA = (InputInstanceNumerical<SubstanceAir::Vec4Float>*)InstanceA;
			InputInstanceNumerical<SubstanceAir::Vec4Float>* InputB = (InputInstanceNumerical<SubstanceAir::Vec4Float>*)InstanceB;

			return
			    FMath::IsNearlyEqual(InputA->getValue().x, InputB->getValue().x, (float)DELTA) &&
			    FMath::IsNearlyEqual(InputA->getValue().y, InputB->getValue().y, (float)DELTA) &&
			    FMath::IsNearlyEqual(InputA->getValue().z, InputB->getValue().z, (float)DELTA) &&
			    FMath::IsNearlyEqual(InputA->getValue().w, InputB->getValue().w, (float)DELTA);
		}
		break;
	case Substance_IOType_Integer:
		{
			InputInstanceNumerical<int32>* InputA = (InputInstanceNumerical<int32>*)InstanceA;
			InputInstanceNumerical<int32>* InputB = (InputInstanceNumerical<int32>*)InstanceB;

			return InputA->getValue() == InputB->getValue();
		}
		break;
	case Substance_IOType_Integer2:
		{
			InputInstanceNumerical<SubstanceAir::Vec2Int>* InputA = (InputInstanceNumerical<SubstanceAir::Vec2Int>*)InstanceA;
			InputInstanceNumerical<SubstanceAir::Vec2Int>* InputB = (InputInstanceNumerical<SubstanceAir::Vec2Int>*)InstanceB;

			return InputA->getValue() == InputB->getValue();
		}
		break;
	case Substance_IOType_Integer3:
		{
			InputInstanceNumerical<SubstanceAir::Vec3Int>* InputA = (InputInstanceNumerical<SubstanceAir::Vec3Int>*)InstanceA;
			InputInstanceNumerical<SubstanceAir::Vec3Int>* InputB = (InputInstanceNumerical<SubstanceAir::Vec3Int>*)InstanceB;

			return InputA->getValue() == InputB->getValue();
		}
		break;
	case Substance_IOType_Integer4:
		{
			InputInstanceNumerical<SubstanceAir::Vec4Int>* InputA = (InputInstanceNumerical<SubstanceAir::Vec4Int>*)InstanceA;
			InputInstanceNumerical<SubstanceAir::Vec4Int>* InputB = (InputInstanceNumerical<SubstanceAir::Vec4Int>*)InstanceB;

			return InputA->getValue() == InputB->getValue();
		}
		break;
	case Substance_IOType_Image:
		{
			SubstanceAir::InputInstanceImage* ImgInputA = (SubstanceAir::InputInstanceImage*)InstanceA;
			SubstanceAir::InputInstanceImage* ImgInputB = (SubstanceAir::InputInstanceImage*)InstanceB;

			return (ImgInputA->getImage() == ImgInputB->getImage()) ? true : false;
		}
		break;
	default:
		break;
	}

	check(0);
	return false;
}

class UnrealSubstanceGlobalCallbacks : public SubstanceAir::GlobalCallbacks
{
public:
	virtual unsigned int getEnabledMask() const override
	{
		return Enable_UserAlloc;
	}

	virtual void* memoryAlloc(size_t bytesCount, size_t alignment) override
	{
#if SUBSTANCE_MEMORY_STAT
		INC_MEMORY_STAT_BY(STAT_SubstanceEngineMemory, bytesCount);
#endif
		return FMemory::Malloc(bytesCount, alignment);
	}

	virtual void memoryFree(void* bufferPtr) override
	{
#if SUBSTANCE_MEMORY_STAT
		SIZE_T size = FMemory::GetAllocSize(bufferPtr);
		DEC_MEMORY_STAT_BY(STAT_SubstanceEngineMemory, size);
#endif

		FMemory::Free(bufferPtr);
	}
};

void SetupSubstance(void* library)
{
	static UnrealSubstanceGlobalCallbacks sGlobalCallbacks;
	SubstanceAir::GlobalCallbacks::setInstance(&sGlobalCallbacks);

	int32 DirectCoreCount = FPlatformMisc::NumberOfCores();
	int32 TotalCoreCount = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
	int32 HyperThreadCount = TotalCoreCount - DirectCoreCount;

	if (GetDefault<USubstanceSettings>()->CPUCores > FPlatformMisc::NumberOfCoresIncludingHyperthreads())
	{
		UE_LOG(LogSubstanceCore, Log, TEXT("The core count substance setting is set to use more cores then are available on this platform."));
		UE_LOG(LogSubstanceCore, Log, TEXT("Physical core count- %d Hyperthread count- %d :: Falling back to use %d cores. (Max available)"),
		       DirectCoreCount, HyperThreadCount, TotalCoreCount);
	}

	GSubstanceDefaultRenderOptions.mCoresCount = FMath::Clamp(GetDefault<USubstanceSettings>()->CPUCores, (int32)1, FPlatformMisc::NumberOfCoresIncludingHyperthreads());
	GSubstanceDefaultRenderOptions.mMemoryBudget = FMath::Clamp(GetDefault<USubstanceSettings>()->MemoryBudgetMb, SBS_MIN_MEM_BUDGET, SBS_MAX_MEM_BUDGET);

	//Note:: We need to convert MB to byte count to pass to initialization
	GSubstanceDefaultRenderOptions.mMemoryBudget *= (1024 * 1024);

	GSubstanceRenderer = TSharedPtr<SubstanceAir::Renderer>(new SubstanceAir::Renderer(GSubstanceDefaultRenderOptions, library));
	GSubstanceRenderer->setRenderCallbacks(gCallbacks.Get());
}

void TearDownSubstance()
{
	LoadingQueue.Empty();
	PriorityLoadingQueue.Empty();
	AsyncQueue.Empty();
	CurrentRenderSet.Empty();
	DelayedImageInputs.Empty();

	GSubstanceRenderer.Reset();
}

void Disable(SubstanceAir::OutputInstance* Output, bool FlagToDelete)
{
	Output->mEnabled = false;

	if (Output->mUserData != 0 && reinterpret_cast<USubstanceOutputData*>(Output->mUserData)->GetData())
	{
		TWeakObjectPtr<UTexture2D> Texture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(Output->mUserData)->GetData());

		//Clear flag allowing texture to be garbage collected
		if (IsGarbageCollecting() && Texture.IsValid() && FlagToDelete)
		{
			//Output->mUserData = 0;
			Texture->ClearFlags(RF_Standalone);
		}
	}
}

const SubstanceAir::GraphDesc* FindParentGraph(const std::vector<SubstanceAir::GraphDesc>& Graphs, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance)
{
	for (const auto& GraphItr : Graphs)
	{
		if (GraphItr.mParent->getUid() == Instance->mInstanceUid)
		{
			return &GraphItr;
		}
	}

	//NOTE:: This should never happen.
	UE_LOG(LogSubstanceCore, Error, TEXT("Unable to find parent graph, returning bad index!"))
	return nullptr;
}

const SubstanceAir::GraphDesc* FindParentGraph(const std::vector<SubstanceAir::GraphDesc>& Graphs, const FString& ParentUrl)
{
	for (const auto& GraphIt : Graphs)
	{
		if (FString(GraphIt.mPackageUrl.c_str()) == ParentUrl)
		{
			return &GraphIt;
		}
	}

	//#NOTE:: This should never happen.
	UE_LOG(LogSubstanceCore, Error, TEXT("Unable to find parent graph, returning bad index!"))
	return nullptr;
}

// Retrieve cooked mipmap of provided texture
void PrepareFileImageInput_GetBGRA(UTexture2D* Input, uint8** Outptr, int32& outWidth, int32& outHeight)
{
	// Stream in texture data
	Input->GetMipData(0, nullptr);

	// Get the mipmap
	if (Input->PlatformData->Mips.Num())
	{
		Input->PlatformData->Mips[0].BulkData.GetCopy((void**)Outptr, false);
		outWidth = Input->GetSizeX();
		outHeight = Input->GetSizeY();
	}
	else
	{
		Outptr = nullptr;
		outWidth = outHeight = 0;
	}
}

SubstanceAir::shared_ptr<SubstanceAir::InputImage> PrepareFileImageInput(UTexture2D* Input)
{
	if (nullptr == Input || UE4FormatToSubstance(Input->GetPixelFormat()) == Substance_PF_Misc)
	{
		// No texture supplied, or it has an unsupported pixel format
		return SubstanceAir::shared_ptr<SubstanceAir::InputImage>();
	}

	// Data used to create the Substance Texture to pass to the engine
	int32 Width = 0;
	int32 Height = 0;
	uint8* UncompressedDataPtr = nullptr;
	SubstancePixelFormat PixelFormat;

#if WITH_EDITORONLY_DATA
	if (Input->Source.IsValid())
	{
		// In editor, we can't access platform data. Let's use texture source instead
		ETextureSourceFormat SourceFormat = Input->Source.GetFormat();
		if (SourceFormat == TSF_BGRA8)
		{
			TArray64<uint8> MipData;
			Input->Source.GetMipData(MipData, 0);
			int32 DataSize = MipData.Num();
			// Copy TArray contents to temporary buffer (just to keep code below consistent)
			UncompressedDataPtr = (uint8*)FMemory::Malloc(DataSize);
			FMemory::Memcpy(UncompressedDataPtr, MipData.GetData(), DataSize);
			// Get remaining parameters of texture
			Width = Input->Source.GetSizeX();
			Height = Input->Source.GetSizeY();
			PixelFormat = Substance_PF_RGBA;
		}
		else if (SourceFormat == TSF_RGBA16)
		{
			TArray64<uint8> MipData;
			Input->Source.GetMipData(MipData, 0);
			int32 DataSize = MipData.Num();
			// Copy TArray contents to temporary buffer (just to keep code below consistent)
			UncompressedDataPtr = (uint8*)FMemory::Malloc(DataSize);
			FMemory::Memcpy(UncompressedDataPtr, MipData.GetData(), DataSize);
			// Get remaining parameters of texture
			Width = Input->Source.GetSizeX();
			Height = Input->Source.GetSizeY();
			PixelFormat = (SubstancePixelFormat)(Substance_PF_RGBA | Substance_PF_16b);
		}
	}
	else
#endif // WITH_EDITORONLY_DATA
	{
		// Use cooked compressed texture
		PrepareFileImageInput_GetBGRA(Input, &UncompressedDataPtr, Width, Height);
		PixelFormat = UE4FormatToSubstance(Input->GetPixelFormat());
	}

	// Create the Substance Texture
	if (UncompressedDataPtr)
	{
		SubstanceTexture texture =
		{
			UncompressedDataPtr,
			(unsigned short)Width,
			(unsigned short)Height,
			(unsigned char)PixelFormat,
			Substance_ChanOrder_BGRA,
			1
		};

		SubstanceAir::shared_ptr<SubstanceAir::InputImage> res = SubstanceAir::InputImage::create(texture);
		// todo: seems this stat is broken - it is not incremented anywhere
#if SUBSTANCE_MEMORY_STAT
		DEC_MEMORY_STAT_BY(STAT_SubstanceImageMemory, FMemory::GetAllocSize(UncompressedDataPtr));
#endif
		FMemory::Free(UncompressedDataPtr);

		return res;
	}
	else
	{
		return SubstanceAir::shared_ptr<SubstanceAir::InputImage>();
	}
}

void SetImageInput(SubstanceAir::InputInstanceImage* Input, UObject* InValue, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Parent, bool unregisterOutput, bool isTransacting)
{
	SubstanceAir::shared_ptr<SubstanceAir::InputImage> NewInputImage = Helpers::PrepareImageInput(InValue, Input, Parent);

	if ((nullptr == InValue && nullptr == NewInputImage.get()) || NewInputImage != Input->getImage())
	{
#if WITH_EDITOR
		if (GIsEditor && isTransacting)
		{
			FScopedTransaction Transaction(NSLOCTEXT("Editor", "ModifiedInput", "Substance"), true);
			reinterpret_cast<GraphInstanceData*>(Input->mParentGraph.mUserData)->ParentGraph->Modify(false);
		}
#endif

		if (NewInputImage.get())
		{
			UTexture2D* InputTexture = Cast<UTexture2D>(InValue);
			NewInputImage->mUserData = (size_t)InputTexture;
		}

		Input->setImage(NewInputImage);
		reinterpret_cast<GraphInstanceData*>(Input->mParentGraph.mUserData)->bHasPendingImageInputRendering = true;
	}
}

SubstanceAir::shared_ptr<SubstanceAir::InputImage> PrepareImageInput(UObject* Image, SubstanceAir::InputInstanceImage* ImgInputInst,
        SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> ImgInputInstParent)
{
	if (Image)
	{
		UTexture2D* BmpImageInput = Cast<UTexture2D>(Image);

		if (BmpImageInput && BmpImageInput->IsValidLowLevel())
		{
			//Link the image input with the input
			return PrepareFileImageInput(BmpImageInput);
		}
	}

	return SubstanceAir::shared_ptr<SubstanceAir::InputImage>();
}

TArray<int32> GetValueInt(const SubstanceAir::InputInstanceBase& Input)
{
	using namespace SubstanceAir;
	TArray<int32> Value;

	switch (Input.mDesc.mType)
	{
	case Substance_IOType_Integer:
		{
			InputInstanceInt* TypedInst = (InputInstanceInt*) & (Input);
			Value.Add(TypedInst->getValue());
		}
		break;
	case Substance_IOType_Integer2:
		{
			InputInstanceInt2* TypedInst = (InputInstanceInt2*) & (Input);
			Value.Add(TypedInst->getValue().x);
			Value.Add(TypedInst->getValue().y);
		}
		break;

	case Substance_IOType_Integer3:
		{
			InputInstanceInt3* TypedInst = (InputInstanceInt3*) & (Input);
			Value.Add(TypedInst->getValue().x);
			Value.Add(TypedInst->getValue().y);
			Value.Add(TypedInst->getValue().z);
		}
		break;

	case Substance_IOType_Integer4:
		{
			InputInstanceInt4* TypedInst = (InputInstanceInt4*) & (Input);
			Value.Add(TypedInst->getValue().x);
			Value.Add(TypedInst->getValue().y);
			Value.Add(TypedInst->getValue().z);
			Value.Add(TypedInst->getValue().w);
		}
		break;
	}

	return Value;
}

FString GetValueString(const SubstanceAir::InputInstanceBase& Input)
{
	if (Input.mDesc.mType != Substance_IOType_String)
	{
		UE_LOG(LogSubstanceCore, Warning, TEXT("Attempted to get the string value of an input that is not a string"))
		return FString();
	}

	using namespace SubstanceAir;
	InputInstanceString* StringInstance = (InputInstanceString*) & (Input);
	return FString(StringInstance->getString().c_str());
}

TArray<float> GetValueFloat(const SubstanceAir::InputInstanceBase& Input)
{
	using namespace SubstanceAir;
	TArray<float> Value;

	switch (Input.mDesc.mType)
	{
	case Substance_IOType_Float:
		{
			InputInstanceFloat* TypedInst = (InputInstanceFloat*) & (Input);
			Value.Add(TypedInst->getValue());
		}
		break;
	case Substance_IOType_Float2:
		{
			InputInstanceFloat2* TypedInst = (InputInstanceFloat2*) & (Input);
			Value.Add(TypedInst->getValue().x);
			Value.Add(TypedInst->getValue().y);
		}
		break;

	case Substance_IOType_Float3:
		{
			InputInstanceFloat3* TypedInst = (InputInstanceFloat3*) & (Input);
			Value.Add(TypedInst->getValue().x);
			Value.Add(TypedInst->getValue().y);
			Value.Add(TypedInst->getValue().z);
		}
		break;

	case Substance_IOType_Float4:
		{
			InputInstanceFloat4* TypedInst = (InputInstanceFloat4*) & (Input);
			Value.Add(TypedInst->getValue().x);
			Value.Add(TypedInst->getValue().y);
			Value.Add(TypedInst->getValue().z);
			Value.Add(TypedInst->getValue().w);
		}
		break;
	}

	return Value;
}

FString GetValueAsString(const SubstanceAir::InputInstanceBase& InInput)
{
	using namespace SubstanceAir;
	FString ValueStr;

	switch ((SubstanceIOType)InInput.mDesc.mType)
	{
	case Substance_IOType_Float:
		{
			InputInstanceFloat* Input = (InputInstanceFloat*)&InInput;
			ValueStr = FString::Printf(TEXT("%f"), Input->getValue());
		}
		break;
	case Substance_IOType_Float2:
		{
			InputInstanceFloat2* Input = (InputInstanceFloat2*)&InInput;
			ValueStr = FString::Printf(TEXT("%f,%f"), Input->getValue().x, Input->getValue().y);
		}
		break;
	case Substance_IOType_Float3:
		{
			InputInstanceFloat3* Input = (InputInstanceFloat3*)&InInput;
			ValueStr = FString::Printf(TEXT("%f,%f,%f"), Input->getValue().x, Input->getValue().y, Input->getValue().z);
		}
		break;
	case Substance_IOType_Float4:
		{
			InputInstanceFloat4* Input = (InputInstanceFloat4*)&InInput;
			ValueStr = FString::Printf(TEXT("%f,%f,%f,%f"), Input->getValue().x, Input->getValue().y, Input->getValue().z, Input->getValue().w);
		}
		break;
	case Substance_IOType_Integer:
		{
			InputInstanceInt* Input = (InputInstanceInt*)&InInput;
			ValueStr = FString::Printf(TEXT("%d"), (int32)Input->getValue());
		}
		break;
	case Substance_IOType_Integer2:
		{
			InputInstanceInt2* Input = (InputInstanceInt2*)&InInput;
			ValueStr = FString::Printf(TEXT("%d,%d"), Input->getValue().x, Input->getValue().y);
		}
		break;
	case Substance_IOType_Integer3:
		{
			InputInstanceInt3* Input = (InputInstanceInt3*)&InInput;
			ValueStr = FString::Printf(TEXT("%d,%d,%d"), Input->getValue().x, Input->getValue().y, Input->getValue().z);
		}
		break;
	case Substance_IOType_Integer4:
		{
			InputInstanceInt4* Input = (InputInstanceInt4*)&InInput;
			ValueStr = FString::Printf(TEXT("%d,%d,%d,%d"), Input->getValue().x, Input->getValue().y, Input->getValue().z, Input->getValue().w);
		}
		break;
	case Substance_IOType_Image:
		{
			SubstanceAir::InputInstanceImage* Input = (SubstanceAir::InputInstanceImage*)&InInput;

			if (Input->getImage())
			{
				reinterpret_cast<UTexture2D*>(Input->getImage()->mUserData)->GetFullName().Split(TEXT(" "), nullptr, &ValueStr);
			}
			else
			{
				ValueStr = TEXT("NULL");
			}
		}
		break;
	default:
		break;
	}

	return ValueStr;
}

void ClearFromRender(USubstanceGraphInstance* Graph)
{
	//we won't need to remove if not valid
	if (!Graph || !Graph->Instance)
	{
		return;
	}

	//Remove from the various queues
	LoadingQueue.Remove(Graph->Instance);
	PriorityLoadingQueue.Remove(Graph->Instance);
	AsyncQueue.Remove(Graph->Instance);
	CurrentRenderSet.Remove(Graph->Instance);

	//Wait for the renderer to finish its current update.
	if (GSubstanceRenderer.IsValid())
	{
		GSubstanceRenderer->cancelAll();
	}

	//Clear any outputs that were just computed that are about to be destroyed
	for (const auto& OutItr : Graph->Instance->getOutputs())
	{
		Substance::Helpers::GetRenderCallbacks()->clearComputedOutputs(OutItr);
	}
}

void ResetGraphInstanceImageInputs(USubstanceGraphInstance* GraphInstance)
{
	//Removes all of the elements within the array that match the predicate requirements
	DelayedImageInputs.RemoveAll([GraphInstance](const GraphImageInputPair & CurElement)
	{
		return (CurElement.second.get() == GraphInstance->Instance.get());
	});
}

bool IsSupportedImageInput(UObject* CandidateImageInput)
{
	//Make sure the image is valid
	if (!CandidateImageInput || CandidateImageInput->IsPendingKill())
	{
		return false;
	}

	//Make sure the image is a supported object
	if (Cast<UTexture2D>(CandidateImageInput))
	{
		return true;
	}

	return false;
}

void ResetToDefault(SubstanceAir::InputInstanceBase* Input)
{
	Input->reset();
}

void ResetToDefault(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance)
{
	//Set all input values to their default value
	for (const auto& InputItr : Instance->getInputs())
	{
		ResetToDefault(InputItr);

		if (InputItr->mDesc.isImage())
		{
			UpdateInput(Instance, InputItr, nullptr);
		}
	}

	//Flag outputs as dirty so they can be updated with the new inputs
	for (const auto& OutputItr : Instance->getOutputs())
	{
		OutputItr->flagAsDirty();
	}
}

void GetSuitableName(SubstanceAir::OutputInstance* Instance, FString& OutAssetName, FString& OutPackageName, USubstanceGraphInstance* OptionalParentGraph)
{
	//Find our instance
	USubstanceGraphInstance* UEGraphInstance;
	if (OptionalParentGraph != nullptr)
	{
		UEGraphInstance = OptionalParentGraph;
	}
	else
	{
		UEGraphInstance = reinterpret_cast<USubstanceOutputData*>(Instance->mUserData)->ParentInstance;
	}

#if WITH_EDITOR
	check(UEGraphInstance);
	check(UEGraphInstance->Instance);
	const SubstanceAir::GraphDesc& Graph = UEGraphInstance->Instance->mDesc;

	static FName AssetToolsModuleName = FName("AssetTools");
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(AssetToolsModuleName);

	FSubstanceCoreModule& SubstanceModule = FModuleManager::LoadModuleChecked<FSubstanceCoreModule>(TEXT("SubstanceCore"));
	const bool PIE = SubstanceModule.isPie();

	for (const auto& OutputItr : Graph.mOutputs)
	{
		//Look for the original description
		if (OutputItr.mUid == Instance->mDesc.mUid)
		{
			FString mUidString = FString::FromInt(Instance->mDesc.mUid);
			FString PackageName;
			
			if (PIE)
			{
				//Play in Editor materials may share an outer package name and require a more unique identifer 
				FString BaseName = UEGraphInstance->GetName() + TEXT("_") + OutputItr.mIdentifier.c_str() + TEXT("_") + mUidString;
				PackageName = UEGraphInstance->GetOuter()->GetName() + TEXT("_") + BaseName;
			}
			else
			{
				PackageName = UEGraphInstance->GetOuter()->GetName() + TEXT("_") + OutputItr.mIdentifier.c_str();
			}
			OutPackageName = PackageName;
			PackageName.Split(TEXT("/"), nullptr, &(OutAssetName), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			return;
		}
	}

	//This should not happen
	check(0);

	//Still got to find a name, and still got to have it unique
	FString BaseName(TEXT("TEXTURE_NAME_NOT_FOUND"));
	AssetToolsModule.Get().CreateUniqueAssetName(reinterpret_cast<USubstanceOutputData*>(Instance->mUserData)->ParentInstance->GetPathName() + TEXT("/") + BaseName,
		TEXT(""), OutPackageName, OutAssetName);
#else
	const SubstanceAir::GraphDesc& Graph = UEGraphInstance->Instance->mDesc;
	for (const auto& OutputItr : Graph.mOutputs)
	{
		//Look for the original description
		if (OutputItr.mUid == Instance->mDesc.mUid)
		{
			FString BaseName = UEGraphInstance->GetName() + TEXT("_") + OutputItr.mIdentifier.c_str();
			FString PackageName = UEGraphInstance->GetOuter()->GetName() + TEXT("_") + BaseName;
			OutPackageName = PackageName;
			OutAssetName = UEGraphInstance->ParentFactory->GetName();
			return;
		}
	}
#endif
}

USubstanceGraphInstance* DuplicateGraphInstance(USubstanceGraphInstance* SourceGraphInstance)
{
	FString BasePath;
	FString ParentName = SourceGraphInstance->ParentFactory->GetOuter()->GetPathName();

	ParentName.Split(TEXT("/"), &(BasePath), nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	FString AssetNameStr;
	FString PackageNameStr;

#if WITH_EDITOR
	FString Name = ObjectTools::SanitizeObjectName(SourceGraphInstance->Instance->mDesc.mLabel.c_str());
	static FName AssetToolsModuleName = FName("AssetTools");
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(AssetToolsModuleName);
	AssetToolsModule.Get().CreateUniqueAssetName(BasePath + TEXT("/") + Name + TEXT("_INST"), TEXT(""), PackageNameStr, AssetNameStr);
#else
	FString Name = SourceGraphInstance->Instance->mDesc.mLabel.c_str();
	PackageNameStr = BasePath;
#endif

	UObject* InstanceParent = CreatePackage(*PackageNameStr);

	USubstanceGraphInstance* NewInstance = Substance::Helpers::InstantiateGraph(SourceGraphInstance->ParentFactory,
	                                       SourceGraphInstance->Instance->mDesc, InstanceParent, AssetNameStr, false, SourceGraphInstance->GetFlags());

	CopyInstance(SourceGraphInstance, NewInstance, false);

#if WITH_EDITOR
	if (NewInstance)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().AssetCreated(NewInstance);
	}
#endif

	return NewInstance;
}

USubstanceGraphInstance* InstantiateGraph(
    USubstanceInstanceFactory* ParentFactory,
    const SubstanceAir::GraphDesc& Graph,
    UObject* Outer,
    FString InstanceName,
    bool bCreateOutputs,
    EObjectFlags Flags,
	UMaterial* InParentMaterial)
{
	USubstanceGraphInstance* GraphInstance = NewObject<USubstanceGraphInstance>(Outer, *InstanceName, Flags);
	GraphInstance->MarkPackageDirty();

	//Set the URL to save for relinking later
	GraphInstance->PackageURL = Graph.mPackageUrl.c_str();
	
	//Register Parent Factory
	GraphInstance->ParentFactory = ParentFactory;
	ParentFactory->RegisterGraphInstance(GraphInstance);

	GraphInstance->CreatedMaterial = InParentMaterial;

	//#TODO:: Use make shared post framework update
	//Create a new instance
	GraphInstance->Instance = SubstanceAir::make_shared<SubstanceAir::GraphInstance>(Graph);

	//Set up the user data for framework access
	GraphInstance->mUserData.ParentGraph = GraphInstance;
	GraphInstance->Instance->mUserData = (size_t)&GraphInstance->mUserData;

	//Create outputs
	if (bCreateOutputs)
	{
		Helpers::CreateTextures(GraphInstance);
#if WITH_EDITORONLY_DATA
		for (SubstanceAir::OutputInstance* outp : GraphInstance->Instance->getOutputs())
		{
			Substance::Helpers::OverwriteSubstancePixelFormatForSourceImage(outp);
		}
#endif
		GraphInstance->MarkPackageDirty();
	}
	return GraphInstance;
}

void CopyInstance(USubstanceGraphInstance* RefInstance, USubstanceGraphInstance* NewInstance, bool bCopyOutputs)
{
#if WITH_EDITOR
	FSubstanceCoreModule& SubstanceModule = FModuleManager::LoadModuleChecked<FSubstanceCoreModule>(TEXT("SubstanceCore"));
	const bool PIE = SubstanceModule.isPie();
#else
	const bool PIE = false;
#endif

	//Copy values from previous
	SubstanceAir::Preset PresetCurrent;
	PresetCurrent.fill(*RefInstance->Instance);
	PresetCurrent.apply(*NewInstance->Instance, SubstanceAir::Preset::Apply_Merge);

	if (bCopyOutputs)
	{
		//Create same outputs as ref instance
		for (uint32 Idx = 0; Idx < NewInstance->Instance->getOutputs().size(); ++Idx)
		{
			SubstanceAir::OutputInstance* OutputRef = RefInstance->Instance->getOutputs()[Idx];
			SubstanceAir::OutputInstance* Output = NewInstance->Instance->getOutputs()[Idx];

			if (OutputRef->mEnabled)
			{
#if WITH_EDITOR
				FString TextureName;
				FString PackageName;
				Substance::Helpers::GetSuitableName(Output, TextureName, PackageName);
#else // WITH_EDITOR
				FString TextureName;
				FString PackageName;
				PackageName = RefInstance->ParentFactory->GetName();
#endif
				UObject* TextureParent = CreatePackage(*PackageName);

				//#Create new texture - If play in editor (PIE), this means the instance is dynamic / transient
				Substance::Helpers::CreateSubstanceTexture2D(Output, PIE ? true : false, TextureName, TextureParent);
			}
			else
			{
				Output->mEnabled = false;
			}
		}
	}
}

void RegisterForDeletion(USubstanceGraphInstance* InstanceContainer)
{
	InstanceContainer->GetOutermost()->FullyLoad();
	local::InstancesToDelete.AddUnique(InstanceContainer);
}

void RegisterForDeletion(USubstanceInstanceFactory* Factory)
{
	Factory->GetOutermost()->FullyLoad();
	local::FactoriesToDelete.AddUnique(Cast<UObject>(Factory));
}

void RegisterForDeletion(UTexture2D* Texture)
{
	if (Texture && Texture->IsValidLowLevel())
	{
		Texture->GetOutermost()->FullyLoad();

		local::TexturesToDelete.AddUnique(Cast<UObject>(Texture));
	}
}

//#NOTE:: This functions purpose is to clear objects and all of their references from within the editor. This is called when textures are disabled and
//need to be deleted or when either an InstanceFactory or GrpahInstance are deleted with outputs/instances.
void PerformDelayedDeletion()
{
	bool bDeletedSomething = false;

#if WITH_EDITOR
	if (local::TexturesToDelete.Num() + local::FactoriesToDelete.Num() + local::InstancesToDelete.Num() == 0)
	{
		return;
	}

	GEditor->Trans->Reset(NSLOCTEXT("UnrealEd", "DeleteSelectedItem", "Delete Selected Item"));
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	//Find all objects the factory has created and queue them for destruction
	for (const auto FactoryItr : local::FactoriesToDelete)
	{
		USubstanceInstanceFactory* Factory = CastChecked<USubstanceInstanceFactory>(FactoryItr);
		Factory->ConditionalPostLoad();

		for (const auto& GraphItr : Factory->GetGraphInstances())
		{
			if (GraphItr && GraphItr->IsValidLowLevel())
			{
				local::InstancesToDelete.AddUnique(GraphItr);
			}
		}
	}

	//Find all textures created from each graph instance and queue them for deletion
	for (const auto& GraphInstanceItr : local::InstancesToDelete)
	{
		USubstanceGraphInstance* CurrentInstance = CastChecked<USubstanceGraphInstance>(GraphInstanceItr);
		CurrentInstance->ConditionalPostLoad();

		if (CurrentInstance->Instance)
		{
			for (const auto& OutputItr : CurrentInstance->Instance->getOutputs())
			{
				if (OutputItr->mUserData != 0)
				{
					UTexture* Texture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(OutputItr->mUserData)->GetData());
					if (Texture && Texture->IsValidLowLevel())
					{
						local::TexturesToDelete.AddUnique(Texture);
					}
				}
			}
		}
		else
		{
			UE_LOG(LogSubstanceCore, Warning, TEXT("Unable to cleanup Substance outputs as Parent SubstanceGraphInstance or Parent SubstanceInstanceFactory is not valid"))
		}
	}

	//Next - Cleanup Graph Instance
	if (local::TexturesToDelete.Num())
		bDeletedSomething |= (ObjectTools::ForceDeleteObjects(local::TexturesToDelete, false) != 0);

	//Next - Cleanup Graph Instance
	if (local::InstancesToDelete.Num())
		bDeletedSomething |= (ObjectTools::ForceDeleteObjects(local::InstancesToDelete, false) != 0);

	//Lastly - Cleanup Instance Factories
	if (local::FactoriesToDelete.Num())
		bDeletedSomething |= (ObjectTools::ForceDeleteObjects(local::FactoriesToDelete, false) != 0);

#endif //WITH_EDITOR

	local::TexturesToDelete.Empty();
	local::InstancesToDelete.Empty();
	local::FactoriesToDelete.Empty();
}

UPackage* CreateObjectPackage(UObject* Outer, FString ObjectName)
{
	FString Left;
	FString Right;
	FString OuterName = Outer->GetName();
	OuterName.Split(TEXT("/"), &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	FString PkgName = Left + TEXT("/") + ObjectName;
	UPackage* Pkg = CreatePackage(*PkgName);
	Pkg->FullyLoad();
	return Pkg;
}

float GetSubstanceLoadingProgress()
{
	//When no substances are used, return 100%
	if (Substance::GlobalInstancePendingCount == 0)
	{
		return 1.0f;
	}

	return (float)Substance::GlobalInstanceCompletedCount / (float)Substance::GlobalInstancePendingCount;
}

int32 SetImageInputHelper(const SubstanceAir::InputDescBase* InputDesc, UObject* InValue, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance)
{
	int32 ModifiedOuputs = 0;
	SubstanceAir::InputInstanceBase* InputInst = nullptr;

	for (const auto& InputItr : Instance->getInputs())
	{
		if (InputItr->mDesc.mUid == InputDesc->mUid)
		{
			InputInst = InputItr;
			break;
		}
	}

	check(InputInst);
	check(InputInst->mDesc.isImage());

	SubstanceAir::InputInstanceImage* ImageInstance = (SubstanceAir::InputInstanceImage*)(InputInst);
	SetImageInput(ImageInstance, InValue, Instance, true, false);

	if (!reinterpret_cast<GraphInstanceData*>(Instance->mUserData)->bHasPendingImageInputRendering)
	{
		return 0;
	}

	for (const auto& AlteringOutItr : InputDesc->mAlteredOutputUids)
	{
		SubstanceAir::OutputInstance* OutputModified = Instance->findOutput(AlteringOutItr);

		if (OutputModified && OutputModified->mEnabled && OutputModified->mUserData != 0)
		{
			UTexture2D* CurrentTexture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(OutputModified->mUserData)->GetData());
			if (CurrentTexture)
				CurrentTexture->MarkPackageDirty();
			OutputModified->flagAsDirty();
			++ModifiedOuputs;
		}
	}

	TWeakObjectPtr<USubstanceGraphInstance> ParentGraph = reinterpret_cast<GraphInstanceData*>(Instance->mUserData)->ParentGraph;
	if (ModifiedOuputs && ParentGraph.IsValid())
	{
		UTexture2D* Image = Cast<UTexture2D>(InValue);
		ParentGraph->ImageSources.Add(InputDesc->mUid, Image);
		ParentGraph->MarkPackageDirty();
		return ModifiedOuputs;
	}

	return 0;
}

int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, class UObject* InValue)
{
	int32 ModifiedOuputs = 0;

	//Special case for reimported image inputs
	if (!InputInstance)
	{
		for (const auto& InputItr : Instance->getInputs())
		{
			SubstanceAir::InputImage* Input = ((SubstanceAir::InputInstanceImage*)InputItr)->getImage().get();
			if (InputItr->mDesc.isImage() && Input)
			{
				size_t& ImageUserData = Input->mUserData;
				if (reinterpret_cast<UTexture2D*>(ImageUserData) == InValue)
				{
					ModifiedOuputs += SetImageInputHelper(&InputItr->mDesc, InValue, Instance);
				}
			}
		}

		return ModifiedOuputs;
	}

	for (const auto& InputItr : Instance->getInputs())
	{
		if (InputItr->mDesc.mUid == InputInstance->mDesc.mUid)
		{
			ModifiedOuputs += SetImageInputHelper(&InputItr->mDesc, InValue, Instance);
		}
	}

	return ModifiedOuputs;
}

int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, const TArray<int32>& InValue)
{
	return UpdateInputHelper(Instance, InputInstance->mDesc.mUid);
}

int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, const TArray<float>& InValue)
{
	return UpdateInputHelper(Instance, InputInstance->mDesc.mUid);
}

int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, const FString& InValue)
{
	return UpdateInputHelper(Instance, InputInstance->mDesc.mUid);
}

int32 UpdateInputHelper(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const uint32& Uid)
{
	const SubstanceAir::GraphDesc& ParentGraph = Instance->getOutputs()[0]->mParentGraph.mDesc;

	int32 ModifiedOuputs = 0;
	for (const auto& InputItr : ParentGraph.mInputs)
	{
		if (InputItr->mUid == Uid)
		{
			if (InputItr->isNumerical())
			{
				for (unsigned int mAlteredOutputUid : InputItr->mAlteredOutputUids)
				{
					SubstanceAir::OutputInstance* Output = Instance->findOutput(mAlteredOutputUid);
					
					if (Output && Output->mEnabled)
					{
						Output->invalidate();
						++ModifiedOuputs;
					}
				}
			}
			break;
		}
	}

	return ModifiedOuputs;
}

void SetNumericalInputValue(const SubstanceAir::InputInstanceNumericalBase* BaseInput, const TArray<int>& InValue)
{
	SetNumericalInputValueHelper(BaseInput, InValue);
}

void SetNumericalInputValue(const SubstanceAir::InputInstanceNumericalBase* BaseInput, const TArray<float>& InValue)
{
	SetNumericalInputValueHelper(BaseInput, InValue);
}

SUBSTANCECORE_API void SetStringInputValue(const SubstanceAir::InputInstanceBase* Input, const FString& InValue)
{
	using namespace SubstanceAir;
	if (Input->mDesc.mType != Substance_IOType_String)
	{
		UE_LOG(LogSubstanceCore, Warning, TEXT("Attempted to set the string value of an input that is not a string input type"))
		return;
	}

	InputInstanceString* StringInstance = (InputInstanceString*)(Input);
	SubstanceAir::string NewValue = SubstanceAir::string(TCHAR_TO_UTF8(*InValue));
	StringInstance->setString(NewValue);
}

template <typename T> void SetNumericalInputValueHelper(const SubstanceAir::InputInstanceNumericalBase* BaseInput, const TArray<T>& InValue)
{
	switch ((SubstanceIOType)BaseInput->mDesc.mType)
	{
	case Substance_IOType_Float:
		{
			SubstanceAir::InputInstanceFloat* Input = (SubstanceAir::InputInstanceFloat*)BaseInput;

			if (InValue.Num() >= 1)
			{
				Input->setValue(InValue[0]);
			}
			break;
		}
	case Substance_IOType_Float2:
		{
			SubstanceAir::InputInstanceFloat2* Input = (SubstanceAir::InputInstanceFloat2*)BaseInput;

			if (InValue.Num() >= 2)
			{
				SubstanceAir::Vec2Float VecToStore;
				VecToStore.x = InValue[0];
				VecToStore.y = InValue[1];
				Input->setValue(VecToStore);
			}
			break;
		}
	case Substance_IOType_Float3:
		{
			SubstanceAir::InputInstanceFloat3* Input = (SubstanceAir::InputInstanceFloat3*)BaseInput;

			if (InValue.Num() >= 3)
			{
				SubstanceAir::Vec3Float VecToStore;
				VecToStore.x = InValue[0];
				VecToStore.y = InValue[1];
				VecToStore.z = InValue[2];
				Input->setValue(VecToStore);
			}
			break;
		}
	case Substance_IOType_Float4:
		{
			SubstanceAir::InputInstanceFloat4* Input = (SubstanceAir::InputInstanceFloat4*)BaseInput;

			if (InValue.Num() >= 4)
			{
				SubstanceAir::Vec4Float VecToStore;
				VecToStore.x = InValue[0];
				VecToStore.y = InValue[1];
				VecToStore.z = InValue[2];
				VecToStore.w = InValue[3];
				Input->setValue(VecToStore);
			}
			break;
		}
	case Substance_IOType_Integer:
		{
			SubstanceAir::InputInstanceInt* Input = (SubstanceAir::InputInstanceInt*)BaseInput;

			if (InValue.Num() >= 1)
			{
				Input->setValue(InValue[0]);
			}
			break;
		}
	case Substance_IOType_Integer2:
		{
			SubstanceAir::InputInstanceInt2* Input = (SubstanceAir::InputInstanceInt2*)BaseInput;

			if (InValue.Num() >= 2)
			{
				SubstanceAir::Vec2Int VecToStore;
				VecToStore.x = InValue[0];
				VecToStore.y = InValue[1];
				Input->setValue(VecToStore);
			}
			break;
		}
	case Substance_IOType_Integer3:
		{
			SubstanceAir::InputInstanceInt3* Input = (SubstanceAir::InputInstanceInt3*)BaseInput;

			if (InValue.Num() >= 3)
			{
				SubstanceAir::Vec3Int VecToStore;
				VecToStore.x = InValue[0];
				VecToStore.y = InValue[1];
				VecToStore.z = InValue[2];
				Input->setValue(VecToStore);
			}
			break;
		}
	case Substance_IOType_Integer4:
		{
			SubstanceAir::InputInstanceInt4* Input = (SubstanceAir::InputInstanceInt4*)BaseInput;

			if (InValue.Num() >= 4)
			{
				SubstanceAir::Vec4Int VecToStore;
				VecToStore.x = InValue[0];
				VecToStore.y = InValue[1];
				VecToStore.z = InValue[2];
				VecToStore.w = InValue[3];
				Input->setValue(VecToStore);
			}
			break;
		}
	default:
		break;
	}
}

//Used by the editor
SubstanceAir::shared_ptr<SubstanceAir::PackageDesc> InstantiatePackage(void* archiveData, uint32 size)
{
	SubstanceAir::shared_ptr<SubstanceAir::PackageDesc> NewPackDesc = SubstanceAir::make_shared<SubstanceAir::PackageDesc>(archiveData, size);
	return NewPackDesc;
}

SUBSTANCECORE_API SubstanceAir::shared_ptr<SubstanceAir::Preset> EmptyPresetAsShared()
{
	return SubstanceAir::shared_ptr<SubstanceAir::Preset>(AIR_NEW(SubstanceAir::Preset));
}

TWeakObjectPtr<UMaterialInstance> CreateMaterial(USubstanceGraphInstance* Instance, const FString& MaterialName, UObject* Outer)
{
#if WITH_EDITOR

	if (Instance->CreatedMaterial == nullptr)
	{
		FString TemplateName = MaterialName + "_Template";
		Instance->CreatedMaterial = GenerateTemplateMaterial(Instance, TemplateName, Outer);
	}

	//Create MaterialInstanceConstant
	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();

	Instance->ConstantCreatedMaterial = (UMaterialInstanceConstant*)Factory->FactoryCreateNew(
		UMaterialInstanceConstant::StaticClass(),
		Substance::Helpers::CreateObjectPackage(Outer, MaterialName),
		*MaterialName,
		RF_Standalone | RF_Public, nullptr, GWarn);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().AssetCreated(Instance->ConstantCreatedMaterial);

	Instance->ConstantCreatedMaterial->SetParentEditorOnly(Instance->CreatedMaterial);

	GenerateMaterialExpressions(Instance->Instance.get(), Instance->ConstantCreatedMaterial, Instance, true);

	Instance->PostEditChange();
	TArray<UObject*> AssetList;
	AssetList.Add(Instance->ConstantCreatedMaterial);

	if (Instance->Instance->mDesc.mPhysicalSize.x != 0 && Instance->Instance->mDesc.mPhysicalSize.y != 0)
		Instance->ConstantCreatedMaterial->SetVectorParameterValueEditorOnly(FMaterialParameterInfo(FName("physicalsize")), FLinearColor(Instance->Instance->mDesc.mPhysicalSize.x, Instance->Instance->mDesc.mPhysicalSize.y, Instance->Instance->mDesc.mPhysicalSize.z, 0));

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	ContentBrowserModule.Get().SyncBrowserToAssets(AssetList, true);

	Instance->ConstantCreatedMaterial->MarkPackageDirty();
#endif
	return Instance->ConstantCreatedMaterial;
}

SUBSTANCECORE_API void GenerateMaterialExpressions(SubstanceAir::GraphInstance* Instance, UMaterialInstance* MaterialInstance, USubstanceGraphInstance* Graph, bool IsOnImport)
{
	if(MaterialInstance == nullptr)
		return;

	TArray<FMaterialParameterInfo> TextureInfoSet;
	TArray<FGuid> TextureGuids;
	MaterialInstance->GetAllTextureParameterInfo(TextureInfoSet, TextureGuids);

	TArray<FMaterialParameterInfo> ScalarInfoSet;
	TArray<FGuid> ScalarGuids;
	MaterialInstance->GetAllScalarParameterInfo(ScalarInfoSet, ScalarGuids);

	TArray<FMaterialParameterInfo> VectorInfoSet;
	TArray<FGuid> VectorGuids;
	MaterialInstance->GetAllVectorParameterInfo(VectorInfoSet, VectorGuids);

	TArray<FMaterialParameterInfo> AllInfoSets;
	AllInfoSets.Append(TextureInfoSet);
	AllInfoSets.Append(ScalarInfoSet);
	AllInfoSets.Append(VectorInfoSet);

	TMap<SubstanceAir::OutputInstance*, FMaterialParameterInfo> MaterialExpressionMap;

	TArray<SubstanceAir::OutputInstance*> UnsetOutputs;
	UnsetOutputs.Append(&Instance->getOutputs()[0], Instance->getOutputs().size());

	//Connect Usage Type to map
	for (int i = 0; i < UnsetOutputs.Num(); ++i)
	{
		for (FMaterialParameterInfo currInfo : AllInfoSets)
		{
			if (FName(UnsetOutputs[i]->mDesc.defaultChannelUseStr().c_str()) == currInfo.Name)
			{
				if (!MaterialExpressionMap.Contains(UnsetOutputs[i]))
				{
					MaterialExpressionMap.Add(UnsetOutputs[i], currInfo);
					UnsetOutputs.RemoveAt(i);
					--i;
					break;
				}
			}
		}
	}
	//Connect identifier to map if Output has not been used and no usage type for it exists
	for (int i = 0; i < UnsetOutputs.Num(); ++i)
	{
		for (FMaterialParameterInfo currInfo : AllInfoSets)
		{
			if (UnsetOutputs[i]->mDesc.mIdentifier.c_str() == currInfo.Name)
			{
				if (!MaterialExpressionMap.Contains(UnsetOutputs[i]))
				{
					MaterialExpressionMap.Add(UnsetOutputs[i], currInfo);
					UnsetOutputs.RemoveAt(i);
					--i;
					break;
				}
			}
		}
	}

	//The template material used did not contain parameter nodes for all of the outputs
	if (UnsetOutputs.Num() && IsOnImport)
	{
		FString OutputWarning = FString("Import Warning- ");
		OutputWarning.Append(FString(Instance->mDesc.mPackageUrl.c_str()));
		OutputWarning.Append(FString("\nCould not find material nodes in template to connect outputs :"));
		for (SubstanceAir::OutputInstance* RemainingOutput : UnsetOutputs)
		{
			OutputWarning.Append(TEXT("\n - "));
			OutputWarning.Append(FString(RemainingOutput->mDesc.mIdentifier.c_str()));
			if (RemainingOutput->mUserData)
			{
				UTexture2D* UnusedTexture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(RemainingOutput->mUserData)->GetData());
				Substance::Helpers::RegisterForDeletion(UnusedTexture);
				Graph->OutputInstances.Remove(RemainingOutput->mDesc.mUid);
				RemainingOutput->mUserData = 0;
			}
		}
		UE_LOG(LogSubstanceCore, Warning, TEXT("%s"), *OutputWarning);
	}

	for (TTuple<SubstanceAir::OutputInstance*, FMaterialParameterInfo> OutputTuple : MaterialExpressionMap)
	{
		USubstanceOutputData* OutputData = reinterpret_cast<USubstanceOutputData*>(OutputTuple.Key->mUserData);
		if (OutputData)
			OutputData->ParamInfo = OutputTuple.Value; // Store the material param info for use later setting values
		SetMaterialExpression(OutputTuple.Key, MaterialInstance, OutputTuple.Value);
	}
}

SUBSTANCECORE_API UMaterial* GenerateTemplateMaterial(USubstanceGraphInstance* Instance, const FString& MaterialName, UObject* Outer)
{
#if WITH_EDITOR
	//Create an unreal material asset
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	UMaterial* UnrealMaterial = nullptr;

	UnrealMaterial = (UMaterial*)MaterialFactory->FactoryCreateNew(
		UMaterial::StaticClass(),
		Substance::Helpers::CreateObjectPackage(Outer, MaterialName),
		*MaterialName,
		RF_Standalone | RF_Public, nullptr, GWarn);

	//Assign the Instance reference to the material
	Instance->CreatedMaterial = UnrealMaterial;

	bool OpacityMapAvailable = false;
	bool RefractionMapAvailable = false;
	bool MetallicMapAvailable = false;
	bool RoughnessMapAvailable = false;

	//Determine which blend mode shader to use for the material based on our available outputs
	for (const auto& OutputItr : Instance->Instance->getOutputs())
	{
		if (OutputItr->mDesc.defaultChannelUse() == SubstanceAir::Channel_Opacity)
			OpacityMapAvailable = true;
		if (OutputItr->mDesc.defaultChannelUse() == SubstanceAir::Channel_Refraction)
			RefractionMapAvailable = true;
		if (OutputItr->mDesc.defaultChannelUse() == SubstanceAir::Channel_Metallic)
			MetallicMapAvailable = true;
		if (OutputItr->mDesc.defaultChannelUse() == SubstanceAir::Channel_Roughness)
			RoughnessMapAvailable = true;
	}

	//Set the material BlendMode in the case we need translucency or masked modes
	if (RefractionMapAvailable)
		UnrealMaterial->BlendMode = EBlendMode::BLEND_Translucent;
	else if (OpacityMapAvailable)
		UnrealMaterial->BlendMode = EBlendMode::BLEND_Masked;

	//Set Lighting Mode if Metal Rough are available
	if (RefractionMapAvailable && MetallicMapAvailable && RoughnessMapAvailable)
		UnrealMaterial->TranslucencyLightingMode = ETranslucencyLightingMode::TLM_Surface;

	//Textures and properties
	for (SubstanceAir::OutputInstance* OutputInst : Instance->Instance->getOutputs())
	{
		CreateMaterialExpression(OutputInst, UnrealMaterial);
	}

	//Special case: emissive only materials
	TArray<FMaterialParameterInfo> ParamInfo;
	TArray<FGuid> ParamIds;
	UnrealMaterial->GetAllTextureParameterInfo(ParamInfo, ParamIds);
	if (ParamInfo.Num() == 1)
	{
		if (ParamInfo[0].Name.ToString() == TEXT("emissive"))
		{
			UnrealMaterial->SetShadingModel(MSM_Unlit);
		}
	}

	//Special case: no roughness but glossiness
	if (!UnrealMaterial->Roughness.IsConnected())
	{
		for (SubstanceAir::OutputInstance* OutputInst : Instance->Instance->getOutputs())
		{
			const SubstanceAir::OutputDesc* Desc = &OutputInst->mDesc;

			//If mUserData is not valid - Output is not supported
			if (!OutputInst->mEnabled)
			{
				continue;
			}

			UTexture* Texture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(OutputInst->mUserData)->GetData());

			if (OutputInst->mDesc.defaultChannelUse() == SubstanceAir::Channel_Glossiness && Texture)
			{
				//Link it to the material
				UMaterialExpressionOneMinus* OneMinus = NewObject<UMaterialExpressionOneMinus>(UnrealMaterial);

				UMaterialExpressionTextureSampleParameter2D* UnrealTextureExpression = CreateSampler(UnrealMaterial, Texture, Desc);

				UnrealTextureExpression->MaterialExpressionEditorX -= 200;
				OneMinus->MaterialExpressionEditorX = -200;
				OneMinus->MaterialExpressionEditorY = UnrealTextureExpression->MaterialExpressionEditorY;

				UnrealTextureExpression->ConnectExpression(OneMinus->GetInput(0), 0);
				UnrealMaterial->Roughness.Expression = OneMinus;

				UnrealMaterial->Expressions.Add(UnrealTextureExpression);
				UnrealMaterial->Expressions.Add(OneMinus);
			}
		}
	}

	//Special case: no roughness and no metallic
	if (!UnrealMaterial->Roughness.IsConnected() && !UnrealMaterial->Metallic.IsConnected())
	{
		for (SubstanceAir::OutputInstance* OutputInst : Instance->Instance->getOutputs())
		{
			//If mUserData is not valid - Output is not supported
			if (!OutputInst->mEnabled)
			{
				continue;
			}

			UTexture* Texture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(OutputInst->mUserData)->GetData());

			if (OutputInst->mDesc.defaultChannelUse() == SubstanceAir::Channel_Specular && Texture)
			{
				UMaterialExpressionTextureSampleParameter2D* UnrealTextureExpression =
					CreateSampler(UnrealMaterial, Texture, &OutputInst->mDesc);

				UnrealMaterial->Specular.Expression = UnrealTextureExpression;

				UnrealMaterial->Expressions.Add(UnrealTextureExpression);
			}
		}
	}

	//Let the material update itself if necessary
	UnrealMaterial->PreEditChange(nullptr);
	UnrealMaterial->PostEditChange();

	return UnrealMaterial;
#else
return nullptr;

#endif //WITH_EDITOR
}

UMaterialExpressionTextureSampleParameter2D* CreateSampler(UMaterial* UnrealMaterial, UTexture* UnrealTexture, const SubstanceAir::OutputDesc* OutputDesc)
{
#if WITH_EDITOR
	UMaterialExpressionTextureSampleParameter2D* UnrealTextureExpression =
		NewObject<UMaterialExpressionTextureSampleParameter2D>(UnrealMaterial);

	UnrealTextureExpression->MaterialExpressionEditorX = -200;
	UnrealTextureExpression->MaterialExpressionEditorY = UnrealMaterial->Expressions.Num() * 180;
	UnrealTextureExpression->Texture = UnrealTexture;
	UnrealTextureExpression->ParameterName = FName(OutputDesc->mIdentifier.c_str());
	UnrealTextureExpression->SamplerType = UnrealTextureExpression->GetSamplerTypeForTexture(UnrealTexture);;

	return UnrealTextureExpression;
#else
	return nullptr;
#endif //WITH_EDITOR
}

void CreateMaterialExpression(SubstanceAir::OutputInstance* OutputInst, UMaterial* UnrealMaterial)
{
#if WITH_EDITOR
	if (!OutputInst->mEnabled)
		return;

	UTexture* Texture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(OutputInst->mUserData)->GetData());

	using namespace SubstanceAir;
	FExpressionInput* MaterialInput = nullptr;

	switch (OutputInst->mDesc.defaultChannelUse())
	{
	case ChannelUse::Channel_BaseColor:
	case ChannelUse::Channel_Diffuse:
		MaterialInput = &UnrealMaterial->BaseColor;
		break;

	case ChannelUse::Channel_Metallic:
		MaterialInput = &UnrealMaterial->Metallic;
		break;

	case ChannelUse::Channel_SpecularLevel:
		MaterialInput = &UnrealMaterial->Specular;
		break;

	case ChannelUse::Channel_Roughness:
		MaterialInput = &UnrealMaterial->Roughness;
		break;

	case ChannelUse::Channel_Refraction:
		MaterialInput = &UnrealMaterial->Refraction;
		break;

	case ChannelUse::Channel_Emissive:
		MaterialInput = &UnrealMaterial->EmissiveColor;
		break;

	case ChannelUse::Channel_Normal:
		MaterialInput = &UnrealMaterial->Normal;
		break;

	case ChannelUse::Channel_Opacity:
		if (UnrealMaterial->BlendMode == EBlendMode::BLEND_Masked)
			MaterialInput = &UnrealMaterial->OpacityMask;
		else if (UnrealMaterial->BlendMode == EBlendMode::BLEND_Translucent)
			MaterialInput = &UnrealMaterial->Opacity;
		break;

	case ChannelUse::Channel_AmbientOcclusion:
		MaterialInput = &UnrealMaterial->AmbientOcclusion;
		break;

	default:
		//Nothing relevant to input, skip it
		return;
		break;
	}

	if (MaterialInput->IsConnected())
	{
		//Slot already used by another output
		return;
	}

	UMaterialExpressionTextureSampleParameter2D* UnrealTextureExpression =
			CreateSampler(UnrealMaterial, Texture, &OutputInst->mDesc);

	UnrealMaterial->Expressions.Add(UnrealTextureExpression);
	MaterialInput->Expression = UnrealTextureExpression;

	TArray<FExpressionOutput> Outputs;
	Outputs = MaterialInput->Expression->GetOutputs();

	FExpressionOutput* Output = &Outputs[0];
	MaterialInput->Mask = Output->Mask;
	MaterialInput->MaskR = Output->MaskR;
	MaterialInput->MaskG = Output->MaskG;
	MaterialInput->MaskB = Output->MaskB;
	MaterialInput->MaskA = Output->MaskA;

#endif //WITH_EDITOR
}

/** This is used to reset material texture parameter values post reimport of a graph instance */
void ResetMaterialTexturesFromGraph(USubstanceGraphInstance* Graph, UMaterialInstanceConstant* OwnedMaterial, const TArray<MaterialParameterSet>& Materials)
{
#if WITH_EDITOR
	for (const auto& MatItr : Materials)
	{
		for (const auto& OutItr : Graph->Instance->getOutputs())
		{
			//Check to see if this output is referenced in the current material
			if (MatItr.ParameterNames.Contains(OutItr->mDesc.mIdentifier.c_str()) && OutItr->mUserData)
			{
				//Loop through all of the expressions - Check if they are a texture sampler and make sure it is
				//sampler that lines up with this output.
				UTexture* Texture = (UTexture2D*)reinterpret_cast<USubstanceOutputData*>(OutItr->mUserData)->GetData();

				//Assign the previous referenced slot back to the recreated output
				UMaterialExpressionTextureSample* TextureSampler = MatItr.ParameterNames[OutItr->mDesc.mIdentifier.c_str()];
				if (Texture && Texture->IsValidLowLevel() && TextureSampler)
					TextureSampler->Texture = Texture;
			}
		}

		//Let the material update itself if necessary
		if (MatItr.Material && MatItr.Material->IsValidLowLevel())
		{
			MatItr.Material->PreEditChange(nullptr);
			MatItr.Material->PostEditChange();
		}
	}

	//Reset the material created when the graph was created
	Graph->ConstantCreatedMaterial = OwnedMaterial;

	GenerateMaterialExpressions(Graph->Instance.get(), Graph->ConstantCreatedMaterial, Graph);

#endif //WITH_EDITOR
}

//Used to reset material instance parameter references to substance outputs post reimport
void ResetMaterialInstanceTexturesFromGraph(USubstanceGraphInstance* Graph, const TArray<MaterialInstanceParameterSet>& Materials)
{
#if WITH_EDITOR
	for (const auto& MatItr : Materials)
	{
		for (const auto& OutItr : Graph->Instance->getOutputs())
		{
			if (MatItr.ParameterNames.Contains(OutItr->mDesc.mIdentifier.c_str()) && OutItr->mUserData)
			{
				UTexture* Texture = Cast<UTexture2D>(reinterpret_cast<USubstanceOutputData*>(OutItr->mUserData)->GetData());

				for (int32 ParameterIndex = 0; ParameterIndex < MatItr.MaterialInstance->TextureParameterValues.Num(); ++ParameterIndex)
				{
					if (MatItr.MaterialInstance->TextureParameterValues[ParameterIndex].ParameterInfo.Name == MatItr.ParameterNames[OutItr->mDesc.mIdentifier.c_str()])
					{
						MatItr.MaterialInstance->TextureParameterValues[ParameterIndex].ParameterValue = Texture;
					}
				}
			}
		}
		//Let the material update itself if necessary
		if (MatItr.MaterialInstance && MatItr.MaterialInstance->IsValidLowLevel())
		{
			MatItr.MaterialInstance->PreEditChange(nullptr);
			MatItr.MaterialInstance->PostEditChange();
		}
	}
#endif //WITH_EDITOR
}

void SetMaterialExpression(SubstanceAir::OutputInstance* OutputInst, UMaterialInstance* MaterialInstance, FMaterialParameterInfo &Info)
{
	//Early out when the output instance is not active
	if (OutputInst->mUserData == 0 || MaterialInstance == 0)
	{
		return;
	}

	USubstanceOutputData* OutputData = reinterpret_cast<USubstanceOutputData*>(OutputInst->mUserData);

	SubstanceAir::OutputInstance::Result result = Substance::Helpers::getNewestOutputResult(OutputInst);
	SubstanceAir::RenderResultNumericalBase* resultNumerical = static_cast<SubstanceAir::RenderResultNumericalBase*>(result.get());

#if WITH_EDITOR
	switch (OutputInst->mDesc.mType)
	{
	case Substance_IOType_Float:
		if (resultNumerical != nullptr)
		{
			((UMaterialInstanceConstant*)MaterialInstance)->SetScalarParameterValueEditorOnly(Info, ((SubstanceAir::RenderResultFloat*)resultNumerical)->mValue);
		}
		break;
	case Substance_IOType_Integer:
		if (resultNumerical != nullptr)
		{
			((UMaterialInstanceConstant*)MaterialInstance)->SetScalarParameterValueEditorOnly(Info, ((SubstanceAir::RenderResultInt*)resultNumerical)->mValue);
		}
		break;
	case Substance_IOType_Float2:
	case Substance_IOType_Float3:
	case Substance_IOType_Float4:
	case Substance_IOType_Integer2:
	case Substance_IOType_Integer3:
	case Substance_IOType_Integer4:
		if (resultNumerical != nullptr)
		{
			((UMaterialInstanceConstant*)MaterialInstance)->SetVectorParameterValueEditorOnly(Info, SubstanceIOVectorToParameter(resultNumerical));
		}
		break;
	case Substance_IOType_Image:
		((UMaterialInstanceConstant*)MaterialInstance)->SetTextureParameterValueEditorOnly(Info, (UTexture*)OutputData->GetData());
		break;
	default:
		break;
	}
#else
	switch (OutputInst->mDesc.mType)
	{
	case Substance_IOType_Float:
		if (resultNumerical != nullptr)
		{
			((UMaterialInstanceDynamic*)MaterialInstance)->SetScalarParameterValue(Info.Name, ((SubstanceAir::RenderResultFloat*)resultNumerical)->mValue);
		}
		break;
	case Substance_IOType_Integer:
		if (resultNumerical != nullptr)
		{
			((UMaterialInstanceDynamic*)MaterialInstance)->SetScalarParameterValue(Info.Name, ((SubstanceAir::RenderResultInt*)resultNumerical)->mValue);
		}
		break;
	case Substance_IOType_Float2:
	case Substance_IOType_Float3:
	case Substance_IOType_Float4:
	case Substance_IOType_Integer2:
	case Substance_IOType_Integer3:
	case Substance_IOType_Integer4:
		if (resultNumerical != nullptr)
		{
			((UMaterialInstanceDynamic*)MaterialInstance)->SetVectorParameterValue(Info.Name, SubstanceIOVectorToParameter(resultNumerical));
		}
		break;
	case Substance_IOType_Image:
		((UMaterialInstanceDynamic*)MaterialInstance)->SetTextureParameterValue(Info.Name, (UTexture*)OutputData->GetData());
		break;
	default:
		break;
	}
#endif //WITH_EDITOR

}

SUBSTANCECORE_API FLinearColor SubstanceIOVectorToParameter(SubstanceAir::RenderResultNumericalBase* NumericOutput)
{
	FLinearColor OutColor = FLinearColor(0,0,0,0);
	switch (NumericOutput->mType)
	{

	case Substance_IOType_Float2:
		OutColor.R = ((SubstanceAir::RenderResultFloat2*)NumericOutput)->mValue.x;
		OutColor.G = ((SubstanceAir::RenderResultFloat2*)NumericOutput)->mValue.y;
		break;
	case Substance_IOType_Float3:
		OutColor.R = ((SubstanceAir::RenderResultFloat3*)NumericOutput)->mValue.x;
		OutColor.G = ((SubstanceAir::RenderResultFloat3*)NumericOutput)->mValue.y;
		OutColor.B = ((SubstanceAir::RenderResultFloat3*)NumericOutput)->mValue.z;
		break;
	case Substance_IOType_Float4:
		OutColor.R = ((SubstanceAir::RenderResultFloat4*)NumericOutput)->mValue.x;
		OutColor.G = ((SubstanceAir::RenderResultFloat4*)NumericOutput)->mValue.y;
		OutColor.B = ((SubstanceAir::RenderResultFloat4*)NumericOutput)->mValue.z;
		OutColor.A = ((SubstanceAir::RenderResultFloat4*)NumericOutput)->mValue.w;
		break;

	case Substance_IOType_Integer2:
		OutColor.R = ((SubstanceAir::RenderResultInt2*)NumericOutput)->mValue.x;
		OutColor.G = ((SubstanceAir::RenderResultInt2*)NumericOutput)->mValue.y;
		break;
	case Substance_IOType_Integer3:
		OutColor.R = ((SubstanceAir::RenderResultInt3*)NumericOutput)->mValue.x;
		OutColor.G = ((SubstanceAir::RenderResultInt3*)NumericOutput)->mValue.y;
		OutColor.B = ((SubstanceAir::RenderResultInt3*)NumericOutput)->mValue.z;
		break;
	case Substance_IOType_Integer4:
		OutColor.R = ((SubstanceAir::RenderResultInt4*)NumericOutput)->mValue.x;
		OutColor.G = ((SubstanceAir::RenderResultInt4*)NumericOutput)->mValue.y;
		OutColor.B = ((SubstanceAir::RenderResultInt4*)NumericOutput)->mValue.z;
		OutColor.A = ((SubstanceAir::RenderResultInt4*)NumericOutput)->mValue.w;
		break;
	default:
		break;

	}
	return OutColor;
}

void CopyPresetData(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Source, SubstanceAir::Preset& Dest)
{
	Dest.fill(*Source);
}

void ApplyPresetData(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Destination, SubstanceAir::Preset& Source)
{
	Source.apply(*Destination);
}

void InstatiateGraphsFromPackage(SubstanceAir::GraphInstances& Instances, SubstanceAir::PackageDesc* Package)
{
	SubstanceAir::instantiate(Instances, *Package);
}

static void* ChooseParentWindowHandle()
{
	void* ParentWindowWindowHandle = nullptr;
#if WITH_EDITOR
	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
	if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
	{
		ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
	}
#endif //WITH_EDITOR
	return ParentWindowWindowHandle;
}

bool SaveFile(const FString& Title, const FString& FileTypes, FString& InOutLastPath, const FString& DefaultFile, FString& OutFilename)
{
#if WITH_EDITOR
	OutFilename = FString();

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool bFileChosen = false;
	TArray<FString> OutFilenames;
	if (DesktopPlatform)
	{
		bFileChosen = DesktopPlatform->SaveFileDialog(
		                  ChooseParentWindowHandle(),
		                  Title,
		                  InOutLastPath,
		                  DefaultFile,
		                  FileTypes,
		                  EFileDialogFlags::None,
		                  OutFilenames
		              );
	}

	bFileChosen = (OutFilenames.Num() > 0);

	if (bFileChosen)
	{
		// User successfully chose a file; remember the path for the next time the dialog opens.
		InOutLastPath = OutFilenames[0];
		OutFilename = OutFilenames[0];
	}

	return bFileChosen;
#else
	return false;
#endif //WITH_EDITOR
}

#if WITH_EDITOR
bool OpenFiles(const FString& Title, const FString& FileTypes, FString& InOutLastPath, EFileDialogFlags::Type DialogMode, TArray<FString>& OutOpenFilenames)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	bool bOpened = false;
	if (DesktopPlatform)
	{
		bOpened = DesktopPlatform->OpenFileDialog(
		              ChooseParentWindowHandle(),
		              Title,
		              InOutLastPath,
		              TEXT(""),
		              FileTypes,
		              DialogMode,
		              OutOpenFilenames
		          );
	}

	bOpened = (OutOpenFilenames.Num() > 0);

	if (bOpened)
	{
		//User successfully chose a file; remember the path for the next time the dialog opens.
		InOutLastPath = OutOpenFilenames[0];
	}

	return bOpened;
}
#endif

SUBSTANCECORE_API FString GetPresetAsXMLString(USubstanceGraphInstance* GraphInstance)
{
	SubstanceAir::Preset CurrentPreset;
	FString Data;

	CurrentPreset.fill(*GraphInstance->Instance);
	Substance::Helpers::WriteSubstanceXmlPreset(CurrentPreset, Data);
	
	return Data;
}

bool ExportPresetFromGraph(USubstanceGraphInstance* GraphInstance)
{
#if WITH_EDITOR
	check(GraphInstance != nullptr);
	SubstanceAir::Preset CurrentPreset;
	FString Data;

	CurrentPreset.fill(*GraphInstance->Instance);

	Substance::Helpers::WriteSubstanceXmlPreset(CurrentPreset, Data);

	FString PresetFileName = Helpers::ExportPresetFile(CurrentPreset.mLabel.c_str());
	if (PresetFileName.Len())
	{
		return FFileHelper::SaveStringToFile(Data, *PresetFileName);
	}
#endif //WITH_EDITOR
	return false;
}

bool ImportAndApplyPresetForGraph(USubstanceGraphInstance* GraphInstance)
{
#if WITH_EDITOR
	FString Data;
	FString PresetFileName;
	SubstanceAir::Presets CurrentPresets;

	PresetFileName = Helpers::ImportPresetFile();

	//Make sure we found a file to import!
	if (PresetFileName.IsEmpty())
	{
		return false;
	}

	FFileHelper::LoadFileToString(Data, *PresetFileName);
	SubstanceAir::parsePresets(CurrentPresets, TCHAR_TO_ANSI(*Data));

	SubstanceAir::Preset CurretPreset = CurrentPresets[0];

	//Apply it the first preset
	if (CurretPreset.apply(*GraphInstance->Instance, SubstanceAir::Preset::Apply_Merge))
	{
		GraphInstance->PrepareOutputsForSave();
		Substance::Helpers::RenderSync(GraphInstance->Instance, true);
		GraphInstance->SaveAllOutputs();
		return true;
	}

#endif //WITH_EDITOR
	return false;
}

FString ExportPresetFile(FString SuggestedFilename)
{
#if WITH_EDITOR
	FString OutOpenFilename;
	FString InOutLastPath = ".";
	if (SaveFile(TEXT("Export Preset"), TEXT("Export Types (*.sbsprs)|*.sbsprs|All Files|*.*"),
	             InOutLastPath,
	             SuggestedFilename + ".sbsprs",
	             OutOpenFilename)
	   )
	{
		return OutOpenFilename;
	}
	else
	{
		return FString();
	}
#else
	return FString();
#endif //WITH_EDITOR
}

FString ImportPresetFile()
{
#if WITH_EDITOR
	TArray<FString> OutOpenFilenames;
	FString InOutLastPath = ".";
	if (OpenFiles(TEXT("Import Preset"), TEXT("Import Types (*.sbsprs)|*.sbsprs|All Files|*.*"),
	              InOutLastPath,
	              EFileDialogFlags::None, //single selection
	              OutOpenFilenames)
	   )
	{
		return OutOpenFilenames[0];
	}
	else
	{
		return FString();
	}
#else
	return FString();
#endif //WITH_EDITOR
}

void ParsePresetAndApply(SubstanceAir::Presets& CurrentPresets, FString& xmlString, USubstanceGraphInstance* Graph)
{
	SubstanceAir::parsePresets(CurrentPresets, TCHAR_TO_ANSI(*xmlString));

	//Defaulting to the highest level
	SubstanceAir::Preset CurrentPreset = CurrentPresets[0];
	CurrentPreset.apply(*Graph->Instance, SubstanceAir::Preset::Apply_Reset);
}

void WriteSubstanceXmlPreset(SubstanceAir::Preset& preset, FString& XmlContent)
{
	XmlContent += TEXT("<sbspresets formatversion=\"1.0\" count=\"1\">");
	XmlContent += TEXT("<sbspreset pkgurl=\"");
	XmlContent += preset.mPackageUrl.c_str();
	XmlContent += TEXT("\" label=\"");
	XmlContent += preset.mLabel.c_str();
	XmlContent += TEXT("\" >\n");

	SubstanceAir::Preset::InputValues::iterator ItV = preset.mInputValues.begin();

	for (; ItV != preset.mInputValues.end(); ++ItV)
	{
		XmlContent += TEXT("<presetinput identifier=\"");
		XmlContent += (*ItV).mIdentifier.c_str();
		XmlContent += TEXT("\" uid=\"");
		XmlContent += FString::Printf(TEXT("%u"), (*ItV).mUid);
		XmlContent += TEXT("\" type=\"");
		XmlContent += FString::Printf(TEXT("%d"), (int32)(*ItV).mType);
		XmlContent += TEXT("\" value=\"");
		XmlContent += (*ItV).mValue.c_str();
		XmlContent += TEXT("\" />\n");
	}

	XmlContent += TEXT("</sbspreset>\n");
	XmlContent += TEXT("</sbspresets>");
}

void ClearCache()
{
	GClearSubstanceCache = true;
}

SUBSTANCECORE_API TArray<UMaterial*> GetSubstanceIncludedMaterials()
{
	TArray<UMaterial*> SubstanceTemplates;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FAssetData> AssetDataArray;

	FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Substance"))->GetMountedAssetPath();
	ContentDir.Append("Templates/parent_materials");

	AssetRegistryModule.Get().GetAssetsByPath(FName(*ContentDir), AssetDataArray);

	for (FAssetData data : AssetDataArray)
	{
		SubstanceTemplates.AddUnique((UMaterial*)data.GetAsset());
	}

	return SubstanceTemplates;
}

//NOTE::This is currently only called through the substance core debug tools.
//This also requires all substance graph instances and textures to be fully loaded to apply the changes.
/** Recomputes all substance graph instances! - Only available from editor */
#if WITH_EDITOR
uint32 RebuildAllSubstanceGraphInstances()
{
#define LOCTEXT_NAMESPACE "SubstanceHelpers"

	//Get a list of all of our graph instances
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().SearchAllAssets(true);
	TArray<FAssetData> AssetData;

	//Load all Material Instances
	const UClass* Class = UMaterialInstanceConstant::StaticClass();
	AssetRegistryModule.Get().GetAssetsByClass(Class->GetFName(), AssetData);
	TArray<UMaterialInstanceConstant*> AllMaterialInstances;
	for (auto& Itr : AssetData)
	{
		UMaterialInstanceConstant * Mat = (UMaterialInstanceConstant*)Itr.GetAsset();
		AllMaterialInstances.AddUnique(Mat);
	}
	AssetData.Empty();

	//Load All Materials
	Class = UMaterial::StaticClass();
	AssetRegistryModule.Get().GetAssetsByClass(Class->GetFName(), AssetData);
	TArray<UMaterial*> AllMaterials;
	for (auto& Itr : AssetData)
	{
		UMaterial * Mat = (UMaterial*)Itr.GetAsset();
		AllMaterials.AddUnique(Mat);
	}
	AssetData.Empty();

	//Load all substancetexture objects
	Class = USubstanceTexture2D::StaticClass();
	AssetRegistryModule.Get().GetAssetsByClass(Class->GetFName(), AssetData);
	TArray<USubstanceTexture2D*> AllSubstanceTextures;
	for (auto& Itr : AssetData)
	{
		AllSubstanceTextures.AddUnique((USubstanceTexture2D*)Itr.GetAsset());
	}
	AssetData.Empty();

	//Load all graph instances but only store ones that need to update
	Class = USubstanceGraphInstance::StaticClass();
	AssetRegistryModule.Get().GetAssetsByClass(Class->GetFName(), AssetData);
	TArray<USubstanceGraphInstance*> GraphsToUpdate;
	for (auto& Itr : AssetData)
	{
		USubstanceGraphInstance * Instance = (USubstanceGraphInstance*)Itr.GetAsset();
		if(Instance->GraphRequiresUpdate())
			GraphsToUpdate.AddUnique(Instance);
	}
	AssetData.Empty();

	UAssetManager& MainAssetManager = UAssetManager::Get();

	TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>> InstancesToRender;

	//Create a slow task for the loading percentage
	FScopedSlowTask SlowTask(GraphsToUpdate.Num(), LOCTEXT("Rebuilding all Substance Graph Instances", "Rebuilding Substance Graphs"));
	SlowTask.MakeDialog();
	UE_LOG(LogSubstanceCore, Log, TEXT("Rebuilding % graph(s)"), GraphsToUpdate.Num());
	//Recompute every graph instance
	for (USubstanceGraphInstance * CurrInstance : GraphsToUpdate)
	{
		//limit the number of rendered substances per update to avoid running out of video memory
		if (InstancesToRender.Num() > MAX_SUBSTANCES_TO_UPDATE)
			break;

		CreateTextures(CurrInstance);
		
		InstancesToRender.AddUnique(CurrInstance->Instance);

		TArray<uint32> OutputsToRemove;
		for (auto& OutputInst : CurrInstance->OutputInstances)
		{
			bool textureUsed = false;

			for (USubstanceTexture2D* TexItr : AllSubstanceTextures)
			{
				if (OutputInst.Key == TexItr->mUid && OutputInst.Value->GetData() && TexItr->ParentInstance == CurrInstance)
				{
					USubstanceTexture2D* Texture = TexItr;
					UTexture2D* GeneratedTexture = Cast<UTexture2D>(OutputInst.Value->GetData());
					GeneratedTexture->MarkPackageDirty();

					//Replace usubstancetexture refernces with utexture2d
					TArray<UObject*> ObjectsConsolidatingTo{ Texture };
					ObjectTools::ConsolidateObjects(GeneratedTexture, ObjectsConsolidatingTo, false);

					//Check material instances for connected usubstancetextures
					for (UMaterialInstanceConstant* MatInst : AllMaterialInstances)
					{
						UMaterialInterface* BaseInst = (UMaterialInterface*)MatInst;
						TArray<FMaterialParameterInfo> PInfoArray;
						TArray<FGuid> GuidArray;
						BaseInst->GetAllTextureParameterInfo(PInfoArray, GuidArray);

						for (FMaterialParameterInfo info : PInfoArray)
						{
							UTexture* texout;
							BaseInst->GetTextureParameterValue(info, texout, false);
							if (texout == Texture)
							{
								MatInst->SetTextureParameterValueEditorOnly(info, GeneratedTexture);
							}
						}
					}

					//Check material objects for connected usubstancetextures
					for (UMaterial* Mat : AllMaterials)
					{
						UMaterialInterface* BaseInst = (UMaterialInterface*)Mat;
						TArray<FMaterialParameterInfo> PInfoArray;
						TArray<FGuid> GuidArray;
						BaseInst->GetAllTextureParameterInfo(PInfoArray, GuidArray);

						for (FMaterialParameterInfo info : PInfoArray)
						{
							UTexture* texout;
							BaseInst->GetTextureParameterValue(info, texout, false);
							if (texout == Texture)
							{
								Mat->SetTextureParameterValueEditorOnly(info.Name, GeneratedTexture);
							}
						}
					}

					textureUsed = true;
					AllSubstanceTextures.Remove(TexItr);
					break;
				}
			}
			if (!textureUsed && OutputInst.Value->GetData())
			{
				for (auto& Output : CurrInstance->Instance->getOutputs())
				{
					if (Output->mDesc.mUid == OutputInst.Key)
					{
						Output->mUserData = 0;
					}
				}
				RegisterForDeletion(Cast<UTexture2D>(OutputInst.Value->GetData()));
				OutputsToRemove.Add(OutputInst.Key);
			}

		}

		for (auto& OutputID : OutputsToRemove)
		{
			CurrInstance->OutputInstances.FindAndRemoveChecked(OutputID);
		}

		//Flag all outputs to be recomputed
		for (const auto& Output : CurrInstance->Instance->getOutputs())
		{
			if (Output && Output->mEnabled)
			{
				//Force update the output
				Output->flagAsDirty();

				USubstanceOutputData* CurrentOutputData = reinterpret_cast<USubstanceOutputData*>(Output->mUserData);
				if (CurrentOutputData && CurrentOutputData->GetData())
				{
					CurrentOutputData->GetData()->MarkPackageDirty();
				}
			}
		}

		//Force a sync render and cache of texture data to prepare new objects for use in the editor
		CurrInstance->PrepareOutputsForSave();
	}

	Substance::Helpers::RenderSync(InstancesToRender, true);

	for (USubstanceGraphInstance * CurrInstance : GraphsToUpdate)
	{
		CurrInstance->SaveAllOutputs(true);

		//Update the progress each loop
		SlowTask.EnterProgressFrame();
	}

	return InstancesToRender.Num();
#undef LOCTEXT_NAMESPACE
}

void CreateDefaultNamedMaterial(USubstanceGraphInstance* Graph, FString Extention)
{
	check(Graph)
	check(Graph->Instance)

	//Create the path to the material by getting the base path of the newly created instance;
	FString MaterialPath;
	FString CurrentFullPath = Graph->GetPathName();
	CurrentFullPath.Split(TEXT("/"), &(MaterialPath), nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	static FName AssetToolsModuleName = FName("AssetTools");
	FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>(AssetToolsModuleName);
	FString NewMaterialName;

	FString InMaterialName(Graph->Instance->mDesc.mLabel.c_str());
	InMaterialName.Append(Extention);

	AssetToolsModule.Get().CreateUniqueAssetName(MaterialPath + TEXT("/"), InMaterialName, MaterialPath, NewMaterialName);

	UObject* MaterialBasePackage = CreatePackage(*MaterialPath);

	Substance::Helpers::CreateMaterial(Graph, NewMaterialName, MaterialBasePackage);
}

SUBSTANCECORE_API bool SubstancesRequireUpdate()
{
	//Get a list of all of our graph instances
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().SearchAllAssets(true);
	TArray<FAssetData> AssetData;
	const UClass* Class = USubstanceGraphInstance::StaticClass();
	AssetRegistryModule.Get().GetAssetsByClass(Class->GetFName(), AssetData);

	TArray<USubstanceGraphInstance*> AllGraphInstances;
	for (auto& Itr : AssetData)
	{
		AllGraphInstances.AddUnique((USubstanceGraphInstance*)Itr.GetAsset());
	}

	for (USubstanceGraphInstance* graph : AllGraphInstances)
	{
		if (graph->GraphRequiresUpdate())
			return true;
	}

	return false;
}

#endif //WITH_EDITOR
} //Namespace Helpers
} //Namespace Substance
