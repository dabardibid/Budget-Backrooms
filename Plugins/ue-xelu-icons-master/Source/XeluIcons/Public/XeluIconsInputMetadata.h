// Copyright 2022 Mickael Daniel. All Rights Reserved.

#pragma once

#include "InputCoreTypes.h"
#include "Engine/DataTable.h"
#include "XeluIconsInputMetadata.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EXeluIconsIconType : uint8
{
	PS4,
	PS5,
	XboxOne,
	XboxSeries,
	Keyboard
};

/** Determine mappings between Keys and Icon textures */
USTRUCT(BlueprintType)
struct XELUICONS_API FXeluIconsInputsMetaData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Xelu Icons")
	FKey Key;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Xelu Icons")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Xelu Icons")
	EXeluIconsIconType IconType = EXeluIconsIconType::Keyboard;
};
