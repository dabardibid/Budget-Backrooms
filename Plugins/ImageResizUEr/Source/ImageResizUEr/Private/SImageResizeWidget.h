// ImageResizUEr
// Plugin code
// Copyright 2015-2022 Turfster / NT Entertainment
// All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "HAL/PlatformFilemanager.h"
#include "LevelEditor.h"
#include "Editor.h"
#include "Misc/FileHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/MessageDialog.h"
#include "UObject/Object.h"
#include "Engine/EngineTypes.h"
#include "Engine/Texture2D.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Dialogs/DlgPickPath.h"
#include "ImageUtils.h"
#include "AssetRegistryModule.h"

#define STB_IMAGE_RESIZE_STATIC
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "ThirdParty/stb_image_resize.h"
#include <Editor/UnrealEd/Classes/Factories/TextureFactory.h>

class SImageResizeWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SImageResizeWidget) {}

	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs);

protected:
	TSharedPtr<SEditableTextBox> importDirBlock;

private:
	bool doReplace = true;	// replace the textures instead of writing them to a new directory?
	bool doUpsample = false; // do we want to allow upsampling?
	bool doDownsample = true; // do we want to allow downsampling?

	FString importDirName;  // where are we importing from?
	unsigned int desiredWidth = 1024;	// texture width the user wants
	unsigned int desiredHeight = 1024;	// texture height the user wants

	// edge options stuff
	TArray<TSharedPtr<FString>> edgeList;
	TSharedPtr<FString> initialEdgeSelected;
	int selectedEdge;

	// filter options stuff
	TArray<TSharedPtr<FString>> filterList;
	TSharedPtr<FString> initialFilterSelected;
	int selectedFilter;

	void OnImportDirNameChanged(const FText& InLabel)	// relic, not really needed with the button version now
	{
		importDirName = InLabel.ToString();
	}

	FText GetImportDirName() const
	{
		return FText::FromString(importDirName);
	}

	TMap<FIntPoint, int> textureCounter;	// for our total texture type list at the ed

	FReply GetImportHeader()	// ... button clicked
	{
		TSharedPtr<SDlgPickPath> PickAssetPathWidget =
			SNew(SDlgPickPath)
			.Title(FText::FromString("Select directory to work on"));

		if (EAppReturnType::Ok == PickAssetPathWidget->ShowModal())
		{
			importDirName = PickAssetPathWidget->GetPath().ToString();
			importDirBlock->SetText(FText::FromString(importDirName));
		}
		return FReply::Handled();
	}

	TArray<FString> ignoreExtensions;	// filename segments we don't want to try to resize

	FText GetIgnoreList() const		// build a single string for display
	{
		FString result = "";
		for (int i = 0; i < ignoreExtensions.Num(); i++)
			result += (ignoreExtensions[i]) + (i == ignoreExtensions.Num() - 1 ? "" : ",");
		return FText::FromString(result);
	}

	void OnIgnoreChanged(const FText& InLabel, ETextCommit::Type TextType)	// user changed the list, rebuild the array
	{
		if (TextType == ETextCommit::OnEnter || TextType == ETextCommit::Default || TextType == ETextCommit::OnUserMovedFocus)
		{
			FString newList = InLabel.ToString();

			newList = newList.Replace(TEXT(" "), TEXT(""));

			ignoreExtensions.Empty();
			newList.ParseIntoArray(ignoreExtensions, TEXT(","), true);
			SaveConfig();
		}
	}

	FText GetFilterTooltip() const	// tooltips for the filter types, based on the stb_image_resize docs
	{
		switch (selectedFilter)
		{
		case 0:
			return FText::FromString("A trapezoid w/1-pixel wide ramps, same result as box for integer scale ratios");
			break;
		case 1:
			return FText::FromString("On upsampling, produces same results as bilinear texture filtering");
			break;
		case 2:
			return FText::FromString("The cubic b-spline (aka Mitchell-Netrevalli with B=1,C=0), gaussian-esque");
			break;
		case 3:
			return FText::FromString("An interpolating cubic spline");
			break;
		case 4:
			return FText::FromString("Mitchell-Netrevalli filter with B=1/3, C=1/3");
			break;
		}
		return FText::FromString("Unknown filter setting?");
	}

	FText GetWidth() const
	{
		return FText::FromString(FString::FromInt(desiredWidth));
	}

	FText GetHeight() const
	{
		return FText::FromString(FString::FromInt(desiredHeight));
	}

	void SetWidth(const FText& InLabel, ETextCommit::Type TextType)
	{
		if (TextType == ETextCommit::OnEnter || TextType == ETextCommit::Default || TextType == ETextCommit::OnUserMovedFocus)
		{
			FString newList = InLabel.ToString();
			if (newList != "")
			{
				int temp = FCString::Atoi(*newList);
				if (temp > 0)
					desiredWidth = temp;
				SaveConfig();
			}
		}
	}

	void SetHeight(const FText& InLabel, ETextCommit::Type TextType)
	{
		if (TextType == ETextCommit::OnEnter || TextType == ETextCommit::Default || TextType == ETextCommit::OnUserMovedFocus)
		{
			FString newList = InLabel.ToString();
			if (newList != "")
			{
				int temp = FCString::Atoi(*newList);
				if (temp > 0)
					desiredHeight = temp;
				SaveConfig();
			}
		}
	}


	void CreateTextureFromImportImage(FImportImage Image, FString ObjectName, FString importDirectory, bool hasAlpha, TEnumAsByte<enum TextureGroup> lodGroup, FString oldPackageName, bool requestSRGB, TextureCompressionSettings requestedCompressionSettings)
	{
		FString NewPackageName = oldPackageName;
		if (oldPackageName == "")
			NewPackageName = TEXT("/Game/") + (importDirectory == "" ? "Textures/" : importDirectory + "/Textures/") + ObjectName;
		else
		{
			int position = NewPackageName.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			NewPackageName.RemoveAt(position, NewPackageName.Len() - position);
		}

		UPackage* Package = CreatePackage(*NewPackageName);
		Package->FullyLoad();
		Package->Modify();

		UTexture2D* Texture = NewObject<UTexture2D>(Package, FName(*ObjectName), RF_Public | RF_Standalone);
		if (Texture)
		{
			Texture->Source.Init(
				Image.SizeX,
				Image.SizeY,
				/*NumSlices=*/ 1,
				Image.NumMips,
				Image.Format,
				Image.RawData.GetData()
			);
			Texture->CompressionSettings = requestedCompressionSettings;

			Texture->SRGB = false;
			Texture->PostEditChange();
			Texture->LODGroup = lodGroup;

			FAssetRegistryModule::AssetCreated(Texture);
			Texture->PostEditChange();

			FString PackageFileName = FPackageName::LongPackageNameToFilename(NewPackageName, FPackageName::GetAssetPackageExtension());
			try
			{
				if (GEditor->SavePackage(Package, Texture, RF_Public | RF_Standalone, *PackageFileName, GError, nullptr, false, true, SAVE_None))
				{
					Package->PostEditChange();
					FAssetRegistryModule::AssetCreated(Texture);
				}
				else
					UE_LOG(LogTemp, Error, TEXT("Could not save package %s"), *PackageFileName);
			}
			catch (...)
			{
				UE_LOG(LogTemp, Error, TEXT("Something went catastrophically wrong trying to save an unreal package for %s. In unreal code."), *ObjectName);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Something went wrong creating a texture for %s."), *ObjectName);
		}
	}
	
	// build a texture with package in the content directory of your choice
	void CreateTexture(int32 width, int32 height, const TArray<FColor>& colorArray, FString ObjectName, FString importDirectory, bool hasAlpha, TEnumAsByte<enum TextureGroup> lodGroup, FString oldPackageName, bool requestSRGB, TextureCompressionSettings requestedCompressionSettings)
	{
		// last minute sanitizing, just in case we missed one
		ObjectName = ObjectName.Replace(TEXT("*"), TEXT("X"));
		ObjectName = ObjectName.Replace(TEXT("?"), TEXT("Q"));
		ObjectName = ObjectName.Replace(TEXT("!"), TEXT("I"));
		ObjectName = ObjectName.Replace(TEXT("."), TEXT("-"));
		ObjectName = ObjectName.Replace(TEXT("&"), TEXT("_"));
		ObjectName = ObjectName.Replace(TEXT(" "), TEXT("_"));

		FString NewPackageName = oldPackageName;
		if (oldPackageName == "")
			NewPackageName = TEXT("/Game/") + (importDirectory == "" ? "Textures/" : importDirectory + "/Textures/") + ObjectName;
		else
		{
			int position = NewPackageName.Find(".", ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			NewPackageName.RemoveAt(position, NewPackageName.Len() - position);
		}
		UPackage* Package = CreatePackage(*NewPackageName);
		Package->FullyLoad();
		Package->Modify();

		FCreateTexture2DParameters FCT;
		FCT.bUseAlpha = hasAlpha;
		FCT.bSRGB = requestSRGB;
		FCT.CompressionSettings = requestedCompressionSettings;

		FString TextureName = ObjectName;
		if (TextureName.Contains("/"))
			TextureName = TextureName.Mid(TextureName.Find("/", ESearchCase::IgnoreCase, ESearchDir::FromEnd) + 1);

		UTexture2D* basetexture = FImageUtils::CreateTexture2D(width, height, colorArray, Package, *TextureName, RF_Public | RF_Standalone, FCT);

		basetexture->LODGroup = lodGroup;

		FAssetRegistryModule::AssetCreated(basetexture);
		basetexture->PostEditChange();

		FString PackageFileName = FPackageName::LongPackageNameToFilename(NewPackageName, FPackageName::GetAssetPackageExtension());
		try
		{
			if (GEditor->SavePackage(Package, basetexture, RF_Public | RF_Standalone, *PackageFileName, GError, nullptr, false, true, SAVE_None))
			{
				Package->PostEditChange();
				FAssetRegistryModule::AssetCreated(basetexture);
			}
			else
				UE_LOG(LogTemp, Error, TEXT("Could not save package %s"), *PackageFileName);
		}
		catch (...)
		{
			UE_LOG(LogTemp, Error, TEXT("Something went catastrophically wrong trying to save an unreal package for %s. In unreal code."), *ObjectName);
		}
	}

	void SetupIgnoreList()	// basic default ignore list
	{
		ignoreExtensions.Empty();
		ignoreExtensions.Add("atlas");
		ignoreExtensions.Add("subuv");
		ignoreExtensions.Add("hdr");
	}

	void LoadConfig()	// load the config data
	{
		TArray<FString> configLines;
		FString configName = FPaths::EngineConfigDir() + "ImageResizUEr.cfg";	// we're saving this engine version-wide, because there's not much to change
		if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*(configName)))
		{
			configLines.Empty();
			FString UTF16test;
			FFileHelper::LoadFileToString(UTF16test, *configName);
			UTF16test = UTF16test.Replace(TEXT("\r"), TEXT(""));
			UTF16test.ParseIntoArray(configLines, TEXT("\n"), false);
			UTF16test.Empty();

			if (configLines.Num() >= 6)
			{
				for (int i = 0; i < configLines.Num(); i++)
				{
					if (configLines[i].StartsWith("IgnoreList="))
					{
						ignoreExtensions.Empty();
						configLines[i].RightChop(configLines[i].Find("=") + 1).ParseIntoArray(ignoreExtensions, TEXT(","), true);
					}
					else
						if (configLines[i].StartsWith("Replace="))
						{
							doReplace = (FCString::Atoi(*configLines[i].RightChop(configLines[i].Find("=") + 1)) == 1);
						}
						else
							if (configLines[i].StartsWith("Upsample="))
							{
								doUpsample = (FCString::Atoi(*configLines[i].RightChop(configLines[i].Find("=") + 1)) == 1);
							}
							else
								if (configLines[i].StartsWith("Downsample="))
								{
									doDownsample = (FCString::Atoi(*configLines[i].RightChop(configLines[i].Find("=") + 1)) == 1);
								}
								else
									if (configLines[i].StartsWith("ResX="))
									{
										desiredWidth = (FCString::Atoi(*configLines[i].RightChop(configLines[i].Find("=") + 1)));
									}
									else
										if (configLines[i].StartsWith("ResY="))
										{
											desiredHeight = (FCString::Atoi(*configLines[i].RightChop(configLines[i].Find("=") + 1)));
										}
										else
											if (configLines[i].StartsWith("Edge="))
											{
												selectedEdge = (FCString::Atoi(*configLines[i].RightChop(configLines[i].Find("=") + 1)));
												if (edgeList.Num() > 0)
												{
													initialEdgeSelected = edgeList[selectedEdge];
												}
												else
												{
													UE_LOG(LogTemp, Error, TEXT("Edge list empty. This should NEVER happen!!!"));
												}
											}
											else
												if (configLines[i].StartsWith("Filter="))
												{
													selectedFilter = (FCString::Atoi(*configLines[i].RightChop(configLines[i].Find("=") + 1)));
													if (filterList.Num() > 0)
													{
														initialFilterSelected = filterList[selectedFilter];
													}
													else
													{
														UE_LOG(LogTemp, Error, TEXT("Filter list empty. This should NEVER happen!!!"));
													}
												}
				}
			}
			else
			{
				SetupIgnoreList();
			}
		}
		else
			SetupIgnoreList();
	}

	void SaveConfig()	// save our user changeables
	{
		FString SaveText = "";
		FString configName = FPaths::EngineConfigDir() + "ImageResizUEr.cfg";

		SaveText += "ResX=" + FString::FromInt(desiredWidth) + "\r\n";
		SaveText += "ResY=" + FString::FromInt(desiredHeight) + "\r\n";
		SaveText += "Edge=" + FString::FromInt(selectedEdge) + "\r\n";
		SaveText += "Filter=" + FString::FromInt(selectedFilter) + "\r\n";
		SaveText += "Replace=" + FString::FromInt((int)doReplace) + "\r\n";
		SaveText += "Upsample=" + FString::FromInt((int)doUpsample) + "\r\n";
		SaveText += "Downsample=" + FString::FromInt((int)doDownsample) + "\r\n";

		FString ignoreListString = "";
		for (int i = 0; i < ignoreExtensions.Num(); i++)
		{
			ignoreListString += (ignoreExtensions[i]) + (i == ignoreExtensions.Num() - 1 ? "" : ",");
		}
		SaveText += "IgnoreList=" + ignoreListString;

		FFileHelper::SaveStringToFile(SaveText, *configName);
	}

	FText HandleEdgeText() const
	{
		return FText::FromString(*edgeList[selectedEdge]);
	}

	void HandleEdgeSelectionChanged(TSharedPtr<FString> inSelection, ESelectInfo::Type SelectInfo)
	{
		if (inSelection.IsValid())
		{
			selectedEdge = edgeList.IndexOfByKey(inSelection);
			SaveConfig();
		}
	}

	FText HandleFilterText() const
	{
		return FText::FromString(*filterList[selectedFilter]);
	}

	void HandleFilterSelectionChanged(TSharedPtr<FString> inSelection, ESelectInfo::Type SelectInfo)
	{
		if (inSelection.IsValid())
		{
			selectedFilter = filterList.IndexOfByKey(inSelection);
			SaveConfig();
		}
	}

	TSharedRef<SWidget> HandleComboGenerateWidget(TSharedPtr<FString> inItem)
	{
		return SNew(STextBlock)
			.Text(FText::FromString(*inItem));
	}

	ECheckBoxState GetInPlace() const
	{
		return doReplace ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void OnInPlaceChanged(ECheckBoxState NewState)
	{
		doReplace = (NewState == ECheckBoxState::Checked);
		SaveConfig();
	}

	ECheckBoxState GetUpSample() const
	{
		return doUpsample ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void OnUpSampleChanged(ECheckBoxState NewState)
	{
		doUpsample = (NewState == ECheckBoxState::Checked);
		SaveConfig();
	}

	ECheckBoxState GetDownSample() const
	{
		return doDownsample ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}

	void OnDownSampleChanged(ECheckBoxState NewState)
	{
		doDownsample = (NewState == ECheckBoxState::Checked);
		SaveConfig();
	}

	int skipCount = 0;

	void DoTexture(const FAssetData& myAssetData)	// this is our main worker function
	{
		UTexture2D* workTexture = (UTexture2D*)myAssetData.GetAsset();	// grab the texture2D

		if (!workTexture)
			return;

		if (workTexture->PlatformData == NULL)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping %s, probably unsupported HDR format?"), *workTexture->GetName());
			skipCount++;
			return;
		}

		if (ignoreExtensions.Num() > 0)	// test against the ignore list
		{
			FString skipTest = workTexture->GetName();

			for (int i = 0; i < ignoreExtensions.Num(); i++)
			{
				if (skipTest.Contains(ignoreExtensions[i]))
				{
					UE_LOG(LogTemp, Warning, TEXT("Skipping %s because it matches ignore list \"%s\""), *skipTest, *ignoreExtensions[i]);
					skipCount++;
					return;
				}
			}
		}

		uint8 wasSRGB = workTexture->SRGB;

		// Okay, do we still have a shot at this? I have no idea what these will do, or if they overlap the platformdata being null case. 
		// Best case scenario, they return BGRA data like the rest, worst case scenario, they'll crash the engine. Due to lack of data... users will have to tell me.
		if ((workTexture->PlatformData->PixelFormat == EPixelFormat::PF_A1) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_A16B16G16R16) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_A2B10G10R10) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_A32B32G32R32F) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_A8) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_FloatR11G11B10) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_FloatRGB) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_FloatRGBA) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_G16) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_G16R16) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_G16R16F) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_G16R16F_FILTER) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_G32R32F) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_G8) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_R16F) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_R16F_FILTER) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_R16G16B16A16_SINT) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_R16G16B16A16_UINT) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_R16G16_UINT) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_R16_SINT) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_R16_UINT) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_R32G32B32A32_UINT) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_R32_FLOAT) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_R32_SINT) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_R32_UINT) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_Unknown) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_UYVY) ||
			(workTexture->PlatformData->PixelFormat == EPixelFormat::PF_ShadowDepth)
			)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s probably has an unsupported format %i. If everything went fine, tell me please."), *workTexture->GetName(), (int)workTexture->PlatformData->PixelFormat);
			//	return;
		}

		unsigned int dest_height = desiredHeight;
		unsigned int dest_width = desiredWidth;



		int tex_width = workTexture->Source.GetSizeX();
		int tex_height = workTexture->Source.GetSizeY();

		if (tex_height == 1)
		{	
			// specialcase for colour profiles etc
			dest_height = 1;
		}
		else
		{
			if (tex_width != tex_height)	// our input texture isn't square
			{
				float difference = (float)tex_width / (float)tex_height;
				float newdifference = (float)dest_width / (float)dest_height;
				if (!FMath::IsNearlyEqual(newdifference, difference, 0.1f))
					dest_width = dest_height * difference;	// use the destination height as the main metric
			}
		}

		FIntPoint testValue = FIntPoint(tex_width, tex_height);	// Add to the list, or increase the current value of the list
		if (textureCounter.Contains(testValue))
			textureCounter[testValue]++;
		else
			textureCounter.Add(testValue, 1);


		if ((tex_width < (int)dest_width || tex_height < (int)dest_height) && !doUpsample)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping %s, smaller."), *workTexture->GetName());
			skipCount++;
			return;
		}

		if ((tex_width > (int)dest_width || tex_height > (int)dest_height) && !doDownsample)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping %s, larger."), *workTexture->GetName());
			skipCount++;
			return;
		}

		if (tex_width == (int)dest_width || (tex_height > 1 && tex_height == (int)dest_height))	// we don't really have to do anything in this case
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipping %s, same size."), *workTexture->GetName());
			skipCount++;
			return;
		}


		TArray<uint8, FDefaultAllocator64> tempArray;
		workTexture->Source.GetMipData(tempArray, 0);	// grab the source data
		int bytesPerPixel = workTexture->Source.GetBytesPerPixel();

		if (tempArray.Num() != bytesPerPixel * tex_width * tex_height)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s didn't have the expected %i bytes (%i x %i x %i), aborting."), *workTexture->GetName(), bytesPerPixel * tex_width * tex_height, tex_width, tex_height, bytesPerPixel);
			tempArray.Empty();
			skipCount++;
			return;
		}

		TextureCompressionSettings wantedTextureCompressionSettings = workTexture->CompressionSettings;


		stbir_colorspace colorSpace = STBIR_COLORSPACE_SRGB;
		if (wasSRGB == 0)
			colorSpace = STBIR_COLORSPACE_LINEAR;

		ETextureSourceFormat sourceFormat = workTexture->Source.GetFormat();
		if (!(sourceFormat == TSF_G8 || sourceFormat == TSF_G16 || sourceFormat == TSF_BGRA8 || sourceFormat == TSF_BGRE8 || sourceFormat == TSF_RGBA16 || sourceFormat == TSF_RGBA16F || sourceFormat == TSF_RGBA8 || sourceFormat == TSF_RGBE8))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s source format %i is unsupported, aborted."), *workTexture->GetName(),(sourceFormat));
			skipCount++;
			tempArray.Empty();
			return;
		}

		int alphaChannel = 3;
		// is it 16 bit?
		if (sourceFormat == TSF_RGBA16 || sourceFormat == TSF_RGBA16F || sourceFormat == TSF_G16)
		{
			if (bytesPerPixel != 8)
				alphaChannel = STBIR_ALPHA_CHANNEL_NONE;
			if (sourceFormat == TSF_RGBA16 || sourceFormat == TSF_G16)
			{
				try
				{

					stbir_uint16* inputPixels = new stbir_uint16[tempArray.Num() / 2];
					for (int64 i = 0; i < tempArray.Num() / 2; i++)
						inputPixels[i] = tempArray[i * 2] + tempArray[i * 2 + 1] * 256;

					int channels = bytesPerPixel / 2;
					stbir_uint16* out = new stbir_uint16[dest_width * dest_height * channels];

					stbir_resize_uint16_generic(inputPixels, tex_width, tex_height, 0, out, dest_width, dest_height, 0, channels, alphaChannel, 0, (stbir_edge)(selectedEdge + 1), (stbir_filter)(selectedFilter + 1), colorSpace, NULL);
					delete[] inputPixels;

					FImportImage Image;
					Image.SizeX = dest_width;
					Image.SizeY = dest_height;
					Image.NumMips = 1;
					Image.Format = sourceFormat;
					Image.SRGB = false;
					Image.RawData.AddUninitialized((int64)dest_width * dest_height * bytesPerPixel);
					for (unsigned int i = 0; i < dest_width * dest_height * channels; i++)
					{
						Image.RawData[i * 2 + 1] = out[i] / 256;
						Image.RawData[i * 2] = out[i] % 256;
					}
					delete[] out;

					if (doReplace)
					{
						FString textureName = workTexture->GetName();
						FString packageName = workTexture->GetPathName();

						CreateTextureFromImportImage(Image, textureName, "", true, workTexture->LODGroup, packageName, wasSRGB == 1, wantedTextureCompressionSettings);
					}
					else
						CreateTextureFromImportImage(Image, workTexture->GetName(), "resized_images", true, workTexture->LODGroup, "", wasSRGB == 1, wantedTextureCompressionSettings);
				}
				catch (...)
				{
					skipCount++;
					UE_LOG(LogTemp, Warning, TEXT("%s failed when creating a 16 bit image. Aborting."), *workTexture->GetName());
				}
			}
			else
			{
				// This *seems* to work, but the only RGBA16F 'textures' I've found are 1 pixel high IES profiles, so... not much rigorous testing done, since it's really hard to zoom 1200% to compare stuff =p
				try
				{
					float* floatArray = new float[tempArray.Num() / 2];
					for (int64 i = 0; i < tempArray.Num() / 2; i++)
					{
						FFloat16 newFloat = FFloat16();
						// recreate a float16 from our raw byte data
						newFloat.Encoded = tempArray[i * 2] + tempArray[i * 2 + 1] * 256;
						// turn it into a classic float for our resize algo
						floatArray[i] = newFloat.GetFloat();
					}
					int channels = bytesPerPixel / 2;
					float* out = new float[dest_width * dest_height * channels];

					stbir_resize_float_generic(floatArray, tex_width, tex_height, 0, out, dest_width, dest_height, 0, channels, alphaChannel, 0, (stbir_edge)(selectedEdge + 1), (stbir_filter)(selectedFilter + 1), colorSpace, NULL);
					delete[] floatArray;

					FImportImage Image;
					Image.SizeX = dest_width;
					Image.SizeY = dest_height;
					Image.NumMips = 1;
					Image.Format = sourceFormat;
					Image.SRGB = false;
					Image.RawData.AddUninitialized((int64)dest_width* dest_height* bytesPerPixel);

					for (int64 i = 0; i < dest_width * dest_height * channels; i++)
					{
						FFloat16 converted = FFloat16();
						// create a float16 from our resized classic float
						converted.Set(out[i]);
						// rebuild raw byte data from our encoded float16.
						Image.RawData[i * 2] = converted.Encoded % 256;
						Image.RawData[i * 2 + 1] = converted.Encoded / 256;
					}
					delete[] out;

					if (doReplace)
					{
						FString textureName = workTexture->GetName();
						FString packageName = workTexture->GetPathName();

						CreateTextureFromImportImage(Image, textureName, "", true, workTexture->LODGroup, packageName, wasSRGB == 1, wantedTextureCompressionSettings);
					}
					else
						CreateTextureFromImportImage(Image, workTexture->GetName(), "resized_images", true, workTexture->LODGroup, "", wasSRGB == 1, wantedTextureCompressionSettings);
				}
				catch (...)
				{
					skipCount++;
					UE_LOG(LogTemp, Warning, TEXT("%s failed when converting RGBA16F. Aborting."), *workTexture->GetName());
				}
			}
		}
		else
		{
			try
			{
				unsigned char* inputPixels = new unsigned char[tempArray.Num()];
				for (int64 i = 0; i < tempArray.Num(); i++)
					inputPixels[i] = tempArray[i];

				tempArray.Empty();

				if (bytesPerPixel != 4)
					alphaChannel = STBIR_ALPHA_CHANNEL_NONE;

				unsigned char* out = new unsigned char[dest_width * dest_height * bytesPerPixel];	// this is where we want our output data

				stbir_resize_uint8_generic(inputPixels, tex_width, tex_height, 0, out, dest_width, dest_height, 0, bytesPerPixel, alphaChannel, 0, (stbir_edge)(selectedEdge + 1), (stbir_filter)(selectedFilter + 1), colorSpace, NULL);
				delete[] inputPixels;

				int firstIndex = 2;
				int thirdIndex = 0;
				// these are deprecated and *probably* don't exist in any capacity any longer, but... just to be safe I guess?
				if (sourceFormat == TSF_RGBA8 || sourceFormat == TSF_RGBE8)
				{
					thirdIndex = 2;
					firstIndex = 0;
				}

				TArray<FColor> newTextureData;	// rebuild for the UE functions...
				switch (bytesPerPixel)
				{
				case 1:
					for (unsigned int i = 0; i < dest_width * dest_height; i++)
						newTextureData.Add(FColor(out[i], out[i], out[i]));
					break;
				case 2: // this probably definitely isn't a thing, but... eh
					for (unsigned int i = 0; i < dest_width * dest_height; i++)
						newTextureData.Add(FColor(out[i * 2], out[i * 2 + 1], 0));
					break;
				case 3: // so judging by the source formats, we just don't... accept RGB/BGR separately? filling it up anyway
					for (unsigned int i = 0; i < dest_width * dest_height; i++)
						newTextureData.Add(FColor(out[i * 3 + firstIndex], out[i * 3 + 1], out[i * 3 + thirdIndex]));
					break;
				case 4:
					for (unsigned int i = 0; i < dest_width * dest_height; i++)
						newTextureData.Add(FColor(out[i * 4 + firstIndex], out[i * 4 + 1], out[i * 4 + thirdIndex], out[i * 4 + 3])); // GetMipData returns BGRA, but CreateTexture2D wants RGBA
					break;
				}
				delete[] out;

				if (doReplace)
				{
					FString textureName = workTexture->GetName();
					FString packageName = workTexture->GetPathName();

					CreateTexture(dest_width, dest_height, newTextureData, textureName, "", true, workTexture->LODGroup, packageName, wasSRGB == 1, wantedTextureCompressionSettings);
				}
				else
					CreateTexture(dest_width, dest_height, newTextureData, workTexture->GetName(), "resized_images", true, workTexture->LODGroup, "", wasSRGB == 1, wantedTextureCompressionSettings);
			}
			catch (...)
			{
				skipCount++;
				UE_LOG(LogTemp, Warning, TEXT("%s failed when creating an 8 bit image. Aborting."), *workTexture->GetName());
			}
		}
	}

	FReply DoDirectory()
	{
		skipCount = 0;
		SaveConfig();	// always save settings first
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetData> AssetData;
		AssetData.Empty();
		AssetRegistryModule.Get().GetAssetsByClass("Texture", AssetData, true);	// grab all the textures in the asset registry

		TArray<int> foundTextureNames;
		textureCounter.Empty();

		for (int i = 0; i < AssetData.Num(); i++)
		{
			if (AssetData[i].PackageName.ToString().Contains(importDirName))		// pick out the ones that are in the directory (and subdirs) we want
				foundTextureNames.Add(i);
		}

		if (foundTextureNames.Num() > 0)	// we actually have something!
		{
			int total = foundTextureNames.Num();
			FScopedSlowTask SlowTask(total, FText::FromString("Resizing directory contents"));	// show a progress bar
			SlowTask.MakeDialog();


			for (int current = 0; current < total; current++)
			{
				FAssetData myAssetData = AssetData[foundTextureNames[current]];
				SlowTask.EnterProgressFrame(1, FText::FromString("Resizing: " + myAssetData.AssetName.ToString()));
				DoTexture(myAssetData);
			}
			if (skipCount == 0)
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Done."));
			else
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Done, with " + FString::FormatAsNumber(skipCount) + " skips."));
		}
		else
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Nothing in directory!"));

		// show a list of textures by size in the directories we traversed
		TArray<FIntPoint> keys;
		textureCounter.KeySort([](FIntPoint A, FIntPoint B) { return A.Y < B.Y; });

		textureCounter.GetKeys(keys);
		UE_LOG(LogTemp, Warning, TEXT("---------------------------------------------------------------------"));
		UE_LOG(LogTemp, Warning, TEXT("                      LIST OF TEXTURES BY SIZE                       "));
		UE_LOG(LogTemp, Warning, TEXT("---------------------------------------------------------------------"));
		for (int i = 0; i < keys.Num(); i++)
			UE_LOG(LogTemp, Warning, TEXT("%i %ix%i: %i"), i, keys[i].X, keys[i].Y, textureCounter[keys[i]]);
		UE_LOG(LogTemp, Warning, TEXT("---------------------------------------------------------------------"));

		return FReply::Handled();
	}
};