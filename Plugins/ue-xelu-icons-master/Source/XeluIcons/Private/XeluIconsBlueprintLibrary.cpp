// Copyright 2022 Mickael Daniel. All Rights Reserved.

#include "XeluIconsBlueprintLibrary.h"

#include "InputMappingContext.h"
#include "XeluIconsDeveloperSettings.h"
#include "XeluIconsInputMetadata.h"
#include "XeluIconsLog.h"
#include "Engine/Texture2D.h"
#include "GameFramework/InputSettings.h"

const UXeluIconsDeveloperSettings* UXeluIconsBlueprintLibrary::GetPluginSettings()
{
	const UXeluIconsDeveloperSettings* Settings = GetDefault<UXeluIconsDeveloperSettings>();
	check(Settings);
	return Settings;
}

const UDataTable* UXeluIconsBlueprintLibrary::GetIconsDatatable()
{
	const TSoftObjectPtr<UDataTable> DataTable = GetPluginSettings()->IconsDataTable;
	if (!DataTable.IsValid())
	{
		DataTable.LoadSynchronous();
	}

	return DataTable.Get();
}

UTexture2D* UXeluIconsBlueprintLibrary::GetIconForKey(const FKey Key, const EXeluIconsIconType IconPreference)
{
	const TSoftObjectPtr<UTexture2D> Icon = GetSoftIconForKey(Key, IconPreference);
	return Icon.IsValid() ? Icon.Get() : nullptr;
}

TSoftObjectPtr<UTexture2D> UXeluIconsBlueprintLibrary::GetSoftIconForKey(const FKey Key, EXeluIconsIconType IconPreference, const bool bShouldLoadSynchronous)
{
	const UDataTable* IconsDatatable = GetIconsDatatable();

	// Check if passed in key is a keyboard / mouse and force icon preference to keyboard if it is
	const bool bIsGamepadKey = Key.IsGamepadKey();
	if (bIsGamepadKey && IconPreference == EXeluIconsIconType::Keyboard)
	{
		// Fallback to Xbox one icon type when wrong preference given for gamepad key
		IconPreference = EXeluIconsIconType::XboxOne;
	}
	else if (!bIsGamepadKey)
	{
		// Force Keyboard icon type for any key non gamepad
		IconPreference = EXeluIconsIconType::Keyboard;
	}

	const TSoftObjectPtr<UTexture2D> Texture2D = GetIconTextureInternal(Key, IconsDatatable, IconPreference, bShouldLoadSynchronous);
	if (Texture2D.IsNull())
	{
		XI_LOG(Error, TEXT("GetSoftIconForKey: Texture2D soft pointer is null and can never point to a live UObject"))
		return nullptr;
	}

	return Texture2D;
}

UTexture2D* UXeluIconsBlueprintLibrary::GetIconForInputAction(UInputAction* InputAction, UInputMappingContext* MappingContext, const EXeluIconsIconType IconPreference)
{
	if (!InputAction)
	{
		XI_LOG(Error, TEXT("GetIconForInputAction: InputAction is invalid, please prodive one."))
		return nullptr;
	}

	if (!MappingContext)
	{
		XI_LOG(Error, TEXT("GetIconForInputAction: MappingContext is invalid, please prodive one."))
		return nullptr;
	}

	const TSoftObjectPtr<UTexture2D> Icon = GetSoftIconForInputAction(InputAction, MappingContext, IconPreference);
	return Icon.IsValid() ? Icon.Get() : nullptr;
}

TSoftObjectPtr<UTexture2D> UXeluIconsBlueprintLibrary::GetSoftIconForInputAction(UInputAction* InputAction, UInputMappingContext* MappingContext, const EXeluIconsIconType IconPreference, const bool bShouldLoadSynchronous)
{
	TSoftObjectPtr<UTexture2D> MatchingIcon = nullptr;

	if (!InputAction)
	{
		XI_LOG(Error, TEXT("GetSoftIconForInputAction: InputAction is invalid, please prodive one."))
		return MatchingIcon;
	}

	if (!MappingContext)
	{
		XI_LOG(Error, TEXT("GetSoftIconForInputAction: MappingContext is invalid, please prodive one."))
		return MatchingIcon;
	}

	const UDataTable* IconsDatatable = GetIconsDatatable();

	// Search for the first matching mappings in context
	const TArray<FEnhancedActionKeyMapping> Mappings = MappingContext->GetMappings();
	TArray<FEnhancedActionKeyMapping> KeyMappings = Mappings.FilterByPredicate([&InputAction](const FEnhancedActionKeyMapping& Other)
	{
		return Other.Action == InputAction;
	});

	if (KeyMappings.Num() == 0)
	{
		XI_LOG(Error, TEXT("GetSoftIconForInputAction: MappingContext '%s' doesn't define any mapping for '%s' Input Action."), *GetNameSafe(MappingContext), *GetNameSafe(InputAction))
		return MatchingIcon;
	}

	// Iterate over mappings found for the input action and return the first icon matching icon preference
	for (const FEnhancedActionKeyMapping KeyMapping : KeyMappings)
	{
		TSoftObjectPtr<UTexture2D> FoundIcon = GetIconTextureInternal(KeyMapping.Key, IconsDatatable, IconPreference, bShouldLoadSynchronous);
		if (!FoundIcon.IsNull())
		{
			MatchingIcon = FoundIcon;
			break;
		}
	}

	return MatchingIcon;
}

TSoftObjectPtr<UTexture2D> UXeluIconsBlueprintLibrary::GetSoftIconForActionMapping(const FName InputName, const EXeluIconsIconType IconPreference, const bool bShouldLoadSynchronous)
{
	TSoftObjectPtr<UTexture2D> MatchingIcon = nullptr;

	const UInputSettings* InputSettings = UInputSettings::GetInputSettings();

	// First, get all action names and check for validity of passed in input name
	TArray<FName> ActionNames;
	InputSettings->GetActionNames(ActionNames);

	if (!ActionNames.Contains(InputName))
	{
		XI_LOG(Warning, TEXT("GetSoftIconForActionMapping: Input with name '%s' couldn't be found in Input Settings."), *InputName.ToString())
		return MatchingIcon;
	}

	TArray<TSoftObjectPtr<UTexture2D>> Icons = GetSoftIconsForActionMapping(InputName, IconPreference, bShouldLoadSynchronous);

	// Only return the first icon found here, if any matching icons
	if (Icons.IsValidIndex(0))
	{
		MatchingIcon = Icons[0];
	}

	return MatchingIcon;
}

TArray<TSoftObjectPtr<UTexture2D>> UXeluIconsBlueprintLibrary::GetSoftIconsForActionMapping(FName InputName, EXeluIconsIconType IconPreference, const bool bShouldLoadSynchronous)
{
	TArray<TSoftObjectPtr<UTexture2D>> MatchingIcons;

	const UDataTable* IconsDatatable = GetIconsDatatable();
	const UInputSettings* InputSettings = UInputSettings::GetInputSettings();

	// First, get all action names and check for validity of passed in input name
	TArray<FName> ActionNames;
	InputSettings->GetActionNames(ActionNames);

	if (!ActionNames.Contains(InputName))
	{
		XI_LOG(Warning, TEXT("GetSoftIconsForActionMapping: Input with name '%s' couldn't be found in Input Settings."), *InputName.ToString())
		return MatchingIcons;
	}

	const TArray<FInputActionKeyMapping> ActionMappings = InputSettings->GetActionMappings();

	// Find all Action Mappings defining the provided input name
	TArray<FInputActionKeyMapping> FoundActionMappings = ActionMappings.FilterByPredicate([InputName, IconPreference](const FInputActionKeyMapping& Mapping)
	{
		const bool bIsGamepadKey = Mapping.Key.IsGamepadKey();
		const bool bIsValidKey = IconPreference == EXeluIconsIconType::Keyboard && !bIsGamepadKey || IconPreference != EXeluIconsIconType::Keyboard && bIsGamepadKey;

		if (!bIsValidKey)
		{
			return false;
		}

		return Mapping.ActionName == InputName;
	});

	// We have matching actions, find texture for the keys and return that
	if (FoundActionMappings.Num() > 0)
	{
		for (const FInputActionKeyMapping ActionMapping : FoundActionMappings)
		{
			TSoftObjectPtr<UTexture2D> Icon = GetIconTextureInternal(ActionMapping.Key, IconsDatatable, IconPreference, bShouldLoadSynchronous);
			if (!Icon.IsNull())
			{
				MatchingIcons.AddUnique(Icon);
			}
		}
	}

	return MatchingIcons;
}

FXeluIconsActionMappingIcons UXeluIconsBlueprintLibrary::GetSoftIconForActionMappingWithModifiers(const FName InputName, const EXeluIconsIconType IconPreference, const bool bShouldLoadSynchronous)
{
	FXeluIconsActionMappingIcons MatchingMappingIcons = FXeluIconsActionMappingIcons();

	const UInputSettings* InputSettings = UInputSettings::GetInputSettings();

	// First, get all action names and check for validity of passed in input name
	TArray<FName> ActionNames;
	InputSettings->GetActionNames(ActionNames);

	if (!ActionNames.Contains(InputName))
	{
		XI_LOG(Warning, TEXT("GetSoftIconForActionMappingWithModifiers: Input with name '%s' couldn't be found in Input Settings."), *InputName.ToString())
		return FXeluIconsActionMappingIcons();
	}

	TArray<FXeluIconsActionMappingIcons> MappingIcons = GetSoftIconsForActionMappingWithModifiers(InputName, IconPreference, bShouldLoadSynchronous);

	// Only return the first icon found here, if any matching icons
	if (MappingIcons.IsValidIndex(0))
	{
		MatchingMappingIcons = MappingIcons[0];
	}

	return MatchingMappingIcons;
}

TArray<FXeluIconsActionMappingIcons> UXeluIconsBlueprintLibrary::GetSoftIconsForActionMappingWithModifiers(FName InputName, EXeluIconsIconType IconPreference, bool bShouldLoadSynchronous)
{
	TArray<FXeluIconsActionMappingIcons> ActionMappingIcons;

	const UDataTable* IconsDatatable = GetIconsDatatable();
	const UInputSettings* InputSettings = UInputSettings::GetInputSettings();

	// First, get all action names and check for validity of passed in input name
	TArray<FName> ActionNames;
	InputSettings->GetActionNames(ActionNames);

	if (!ActionNames.Contains(InputName))
	{
		XI_LOG(Warning, TEXT("GetSoftIconsForActionMappingWithModifiers: Input with name '%s' couldn't be found in Input Settings."), *InputName.ToString())
		return ActionMappingIcons;
	}

	const TArray<FInputActionKeyMapping> ActionMappings = InputSettings->GetActionMappings();

	// Find all Action Mappings defining the provided input name
	TArray<FInputActionKeyMapping> FoundActionMappings = ActionMappings.FilterByPredicate([InputName, IconPreference](const FInputActionKeyMapping& Mapping)
	{
		const bool bIsGamepadKey = Mapping.Key.IsGamepadKey();
		const bool bIsValidKey = IconPreference == EXeluIconsIconType::Keyboard && !bIsGamepadKey || IconPreference != EXeluIconsIconType::Keyboard && bIsGamepadKey;

		if (!bIsValidKey)
		{
			return false;
		}

		return Mapping.ActionName == InputName;
	});

	// We have matching actions, find texture for the keys and return that
	if (FoundActionMappings.Num() > 0)
	{
		for (const FInputActionKeyMapping ActionMapping : FoundActionMappings)
		{
			FXeluIconsActionMappingIcons MappingIcon;
			MappingIcon.Key = ActionMapping.Key;
			MappingIcon.bAlt = ActionMapping.bAlt;
			MappingIcon.bCmd = ActionMapping.bCmd;
			MappingIcon.bCtrl = ActionMapping.bCtrl;
			MappingIcon.bShift = ActionMapping.bShift;

			MappingIcon.KeyIcon = GetIconTextureInternal(ActionMapping.Key, IconsDatatable, IconPreference, bShouldLoadSynchronous);
			if (MappingIcon.bAlt)
			{
				MappingIcon.AltIcon = GetIconTextureInternal(EKeys::LeftAlt, IconsDatatable, IconPreference, bShouldLoadSynchronous);
			}

			if (MappingIcon.bCmd)
			{
				MappingIcon.CmdIcon = GetIconTextureInternal(EKeys::LeftCommand, IconsDatatable, IconPreference, bShouldLoadSynchronous);
			}

			if (MappingIcon.bCtrl)
			{
				MappingIcon.CtrlIcon = GetIconTextureInternal(EKeys::LeftControl, IconsDatatable, IconPreference, bShouldLoadSynchronous);
			}

			if (MappingIcon.bShift)
			{
				MappingIcon.ShiftIcon = GetIconTextureInternal(EKeys::LeftShift, IconsDatatable, IconPreference, bShouldLoadSynchronous);
			}

			ActionMappingIcons.Add(MappingIcon);
		}
	}

	return ActionMappingIcons;
}

TSoftObjectPtr<UTexture2D> UXeluIconsBlueprintLibrary::GetIconTextureInternal(const FKey Key, const UDataTable* IconsDatatable, const EXeluIconsIconType IconPreference, const bool bShouldLoadSynchronous)
{
	TSoftObjectPtr<UTexture2D> MatchingIcon = nullptr;

	const bool bIsGamepadKey = Key.IsGamepadKey();

	const FString IconStr = UEnum::GetValueAsString(IconPreference).Replace(TEXT("EXeluIconsIconType::"), TEXT(""));
	const FString Context = FString(TEXT("UXeluIconsBlueprintLibrary::LookupInputsTable"));

	const FString RowNameStr = FString::Printf(TEXT("%s.%s"), *IconStr, *Key.ToString());
	FXeluIconsInputsMetaData* InputMetaData = IconsDatatable->FindRow<FXeluIconsInputsMetaData>(FName(*RowNameStr), Context, false);
	if (!InputMetaData)
	{
		return MatchingIcon;
	}

	if (bShouldLoadSynchronous)
	{
		UTexture2D* Icon = InputMetaData->Icon.LoadSynchronous();
		if (!Icon)
		{
			return MatchingIcon;
		}
	}

	if ((IconPreference == EXeluIconsIconType::Keyboard && !bIsGamepadKey) || (IconPreference != EXeluIconsIconType::Keyboard && bIsGamepadKey))
	{
		MatchingIcon = InputMetaData->Icon;
	}

	return MatchingIcon;
}
