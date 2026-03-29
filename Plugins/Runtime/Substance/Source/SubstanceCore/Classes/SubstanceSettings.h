// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceSettings.h

#pragma once
#include "SubstanceInstanceFactory.h"
#include "Materials/Material.h"
#include "SubstanceSettings.generated.h"

#if PLATFORM_ANDROID || PLATFORM_IOS
#define SBS_MIN_MEM_BUDGET (16)
#define SBS_MAX_MEM_BUDGET (256)
#else
#define SBS_MIN_MEM_BUDGET (128)
#define SBS_MAX_MEM_BUDGET (2048)
#endif

UENUM()
enum ESubstanceEngineType
{
	SET_CPU = 0	UMETA(DisplayName = "CPU Engine"),
	SET_GPU = 1 UMETA(DisplayName = "GPU Engine"),
};

UENUM(BlueprintType)
enum EDefaultSubstanceTextureSize
{
	SIZE_0 = 0 UMETA(Hidden),
	SIZE_16 = 4 UMETA(DisplayName = "16"),
	SIZE_32 = 5 UMETA(DisplayName = "32"),
	SIZE_64 = 6 UMETA(DisplayName = "64"),
	SIZE_128 = 7 UMETA(DisplayName = "128"),
	SIZE_256 = 8 UMETA(DisplayName = "256"),
	SIZE_512 = 9 UMETA(DisplayName = "512"),
	SIZE_1024 = 10 UMETA(DisplayName = "1024"),
	SIZE_2048 = 11 UMETA(DisplayName = "2048"),
	SIZE_4096 = 12 UMETA(DisplayName = "4096"),
};

/** Implements the settings for the Substance plugin. */
UCLASS(config = Engine, defaultconfig)
class SUBSTANCECORE_API USubstanceSettings : public UObject
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, Config, Category = "Hardware Budget", meta = (ClampMin = "16", ClampMax = "2048"))
	int32 MemoryBudgetMb;

	UPROPERTY(EditAnywhere, Config, Category = "Hardware Budget", meta = (ClampMin = "1", ClampMax = "32"))
	int32 CPUCores;

	UPROPERTY(EditAnywhere, Config, Category = "Cooking", meta = (ClampMin = "1", ClampMax = "5", DisplayName = "Mip levels count removed during cooking."))
	int32 AsyncLoadMipClip;

	UPROPERTY(EditAnywhere, Config, Category = "Optimization", meta = (ClampMin = "4", ClampMax = "1024"))
	int32 MaxAsyncSubstancesRenderedPerFrame;

	UPROPERTY(EditAnywhere, Config, Category = "Cooking", meta = (DisplayName = "Substance Engine (requires editor restart to take effect.)"))
	TEnumAsByte<ESubstanceEngineType> SubstanceEngine;

	UPROPERTY(EditAnywhere, Config, Category = "Substance Import Settings", meta = (DisplayName = "Default Substance output width"))
	TEnumAsByte<EDefaultSubstanceTextureSize> DefaultSubstanceOutputSizeX;

	UPROPERTY(EditAnywhere, Config, Category = "Substance Import Settings", meta = (DisplayName = "Default Substance output height"))
	TEnumAsByte<EDefaultSubstanceTextureSize> DefaultSubstanceOutputSizeY;

	UPROPERTY(EditAnywhere, Config, Category = "Substance Import Settings", meta = (DisplayName = "Default Substance Material Template", AllowedClasses = "Material,MaterialInstance,MaterialInterface"))
	TSoftObjectPtr<UMaterialInterface> DefaultTemplateMaterial;
};
