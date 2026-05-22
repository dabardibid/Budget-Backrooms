// Copyright 2022 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "XeluIconsDeveloperSettings.generated.h"

UCLASS(Config="Game", defaultconfig, meta=(DisplayName="Xelu Icons"))
class XELUICONS_API UXeluIconsDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UXeluIconsDeveloperSettings();

	/**
	 * Path to the DataTable used to determine mappings between Keys and Icon textures.
	 *
	 * This is set by default to an internal DataTables (that you can find in `/Xelu_Icons/`) that setup texture icons for every keyboard and gamepad key.
	 *
	 * Icons are coming from Xelu's Free Controllers & Keyboard Prompts: https://thoseawesomeguys.com/prompts/
	 *
	 * Thanks to "Nicolae (Xelu) Berbece" and "Those Awesome Guys" to make it available in the public domain licence under Creative Commons 0 (CC0)
	 */
	UPROPERTY(config, EditDefaultsOnly, Category = "Icons", meta = (RequiredAssetDataTags = "RowStructure=/Script/XeluIcons.XeluIconsInputsMetaData"))
	TSoftObjectPtr<UDataTable> IconsDataTable;

	virtual FName GetCategoryName() const override;
};
