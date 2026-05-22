// Copyright 2022 Mickael Daniel. All Rights Reserved.

#pragma once

#include "InputCoreTypes.h"
#include "XeluIconsInputMetadata.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XeluIconsBlueprintLibrary.generated.h"

class UDataTable;
class UInputAction;
class UInputMappingContext;
class UXeluIconsDeveloperSettings;

USTRUCT(BlueprintType)
struct FXeluIconsActionMappingIcons
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = "Xelu Icons")
	FKey Key;

	/** Set to matching icon for the key */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Xelu Icons")
	TSoftObjectPtr<UTexture2D> KeyIcon = nullptr;

	/** true if one of the Shift keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Xelu Icons")
	bool bShift = false;

	/** Set if one of the Shift keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Xelu Icons")
	TSoftObjectPtr<UTexture2D> ShiftIcon = nullptr;

	/** true if one of the Ctrl keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Xelu Icons")
	bool bCtrl = false;

	/** Set if one of the Ctrl keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Xelu Icons")
	TSoftObjectPtr<UTexture2D> CtrlIcon = nullptr;

	/** true if one of the Alt keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Xelu Icons")
	bool bAlt = false;

	/** Set if one of the Alt keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Xelu Icons")
	TSoftObjectPtr<UTexture2D> AltIcon = nullptr;

	/** true if one of the Cmd keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Xelu Icons")
	bool bCmd = false;

	/** Set if one of the Cmd keys must be down when the KeyEvent is received to be acknowledged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Xelu Icons")
	TSoftObjectPtr<UTexture2D> CmdIcon = nullptr;
};


/**
 * Main library interface for Xelu Icons plugin
 */
UCLASS()
class XELUICONS_API UXeluIconsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Returns plugin settings from Editor config */
	static const UXeluIconsDeveloperSettings* GetPluginSettings();

	/** Returns Icons Datatable mapping from plugin settings */
	static const UDataTable* GetIconsDatatable();

	/** Returns matching icon texture for the given key. IconPreference is only relevant for gamepad keys */
	UFUNCTION(BlueprintPure, Category = "Xelu Icons | Key")
	static UTexture2D* GetIconForKey(FKey Key, EXeluIconsIconType IconPreference = EXeluIconsIconType::Keyboard);

	/** Returns matching icon texture as soft object pointer for the given key. IconPreference is only relevant for gamepad keys */
	UFUNCTION(BlueprintPure, Category = "Xelu Icons | Key")
	static TSoftObjectPtr<UTexture2D> GetSoftIconForKey(FKey Key, EXeluIconsIconType IconPreference = EXeluIconsIconType::Keyboard, bool bShouldLoadSynchronous = true);

	/** Returns matching icon texture given the passed in Input Action / Mapping Context, if there is a mapping defined for this configuration */
	UFUNCTION(BlueprintPure, Category = "Xelu Icons | Enhanced Input")
	static UTexture2D* GetIconForInputAction(UInputAction* InputAction, UInputMappingContext* MappingContext, EXeluIconsIconType IconPreference = EXeluIconsIconType::Keyboard);

	/** Returns matching icon texture as soft object pointer given the passed in Input Action / Mapping Context, if there is a mapping defined for this configuration */
	UFUNCTION(BlueprintPure, Category = "Xelu Icons | Enhanced Input")
	static TSoftObjectPtr<UTexture2D> GetSoftIconForInputAction(UInputAction* InputAction, UInputMappingContext* MappingContext, EXeluIconsIconType IconPreference = EXeluIconsIconType::Keyboard, bool bShouldLoadSynchronous = true);

	/** Returns the first matching icon texture as soft object pointer given the passed in Action Name, if there is a mapping defined for it in Input Settings */
	UFUNCTION(BlueprintPure, Category = "Xelu Icons | Input Settings")
	static TSoftObjectPtr<UTexture2D> GetSoftIconForActionMapping(FName InputName, EXeluIconsIconType IconPreference = EXeluIconsIconType::Keyboard, bool bShouldLoadSynchronous = true);

	/** Returns a list of matching icon textures as soft object pointers given the passed in Action Name, if there is one ore more mapping defined for it in Input Settings. */
	UFUNCTION(BlueprintPure, Category = "Xelu Icons | Input Settings")
	static TArray<TSoftObjectPtr<UTexture2D>> GetSoftIconsForActionMapping(FName InputName, EXeluIconsIconType IconPreference = EXeluIconsIconType::Keyboard, bool bShouldLoadSynchronous = true);

	/** Returns a structure with Key and Modifiers textures as soft object pointers given the passed in Action Name, if there is a mapping defined for it in Input Settings */
	UFUNCTION(BlueprintPure, Category = "Xelu Icons | Input Settings")
	static FXeluIconsActionMappingIcons GetSoftIconForActionMappingWithModifiers(FName InputName, EXeluIconsIconType IconPreference = EXeluIconsIconType::Keyboard, bool bShouldLoadSynchronous = true);

	/** Returns a list structure with Key and Modifiers textures as soft object pointers given the passed in Action Name, if there is one or more mapping defined for it in Input Settings */
	UFUNCTION(BlueprintPure, Category = "Xelu Icons | Input Settings")
	static TArray<FXeluIconsActionMappingIcons> GetSoftIconsForActionMappingWithModifiers(FName InputName, EXeluIconsIconType IconPreference = EXeluIconsIconType::Keyboard, bool bShouldLoadSynchronous = true);

	/** Main lookup helper to return the matching icon texture for the passed in key */
	static TSoftObjectPtr<UTexture2D> GetIconTextureInternal(FKey Key, const UDataTable* IconsDatatable, EXeluIconsIconType IconPreference, bool bShouldLoadSynchronous = true);
};
