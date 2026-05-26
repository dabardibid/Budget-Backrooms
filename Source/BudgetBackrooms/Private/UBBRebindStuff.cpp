#include "UBBRebindStuff.h"
#include "GameFramework/InputSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "InputCoreTypes.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "UObject/SoftObjectPath.h"


// The DataTable holding all gamepad icons. Row struct must be FBBKeyIconRow.
// Row names follow "<Style>.<KeyName>" convention (matches Xelu's CSV).
static const FSoftObjectPath GBBIconDataTablePath(
    TEXT("/Game/Widgets/DT_BBKeyIcons.DT_BBKeyIcons"));

// Fallback texture shown when a row lookup fails (unknown key, broken soft ptr,
// missing texture asset). Create this asset in /Game/Widgets/ before first run.
// A 64x64 solid color or a "?" glyph is fine for placeholder;

static const FSoftObjectPath GBBMissingIconTexturePath(
    TEXT("/Game/Widgets/T_BBKeyIcon_Missing.T_BBKeyIcon_Missing"));


FString UBBRebindStuff::GetDefaultInputIniPath()
{
    // FPaths::SourceConfigDir() points to <Project>/Config in editor,
    // and to the staged Config folder in packaged builds. This is where
    // DefaultInput.ini lives - NOT the Saved/Config folder, which holds
    // user-modified overrides.
    return FPaths::SourceConfigDir() / TEXT("DefaultInput.ini");
}

TArray<FInputActionKeyMapping> UBBRebindStuff::GetDefaultActionMappings()
{
    TArray<FInputActionKeyMapping> Result;

    const FString IniPath = GetDefaultInputIniPath();

    FConfigFile ConfigFile;
    ConfigFile.Read(IniPath);

    if (ConfigFile.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("RebindHelper: Could not read DefaultInput.ini at %s"), *IniPath);
        return Result;
    }

    const FConfigSection* Section = ConfigFile.Find(TEXT("/Script/Engine.InputSettings"));
    if (!Section)
    {
        UE_LOG(LogTemp, Warning, TEXT("RebindHelper: No [/Script/Engine.InputSettings] section found"));
        return Result;
    }

    TArray<FConfigValue> RawValues;
    Section->MultiFind(TEXT("ActionMappings"), RawValues, /*bMaintainOrder=*/true);

    for (const FConfigValue& RawValue : RawValues)
    {
        const FString& Line = RawValue.GetValue();

        FInputActionKeyMapping Mapping;
        const TCHAR* ImportPtr = *Line;
        if (FInputActionKeyMapping::StaticStruct()->ImportText(
            ImportPtr, &Mapping, nullptr, PPF_None, GLog, TEXT("FInputActionKeyMapping")))
        {
            Result.Add(Mapping);
        }
    }

    return Result;
}

TArray<FInputAxisKeyMapping> UBBRebindStuff::GetDefaultAxisMappings()
{
    TArray<FInputAxisKeyMapping> Result;

    const FString IniPath = GetDefaultInputIniPath();

    FConfigFile ConfigFile;
    ConfigFile.Read(IniPath);

    if (ConfigFile.Num() == 0)
    {
        return Result;
    }

    const FConfigSection* Section = ConfigFile.Find(TEXT("/Script/Engine.InputSettings"));
    if (!Section)
    {
        return Result;
    }

    TArray<FConfigValue> RawValues;
    Section->MultiFind(TEXT("AxisMappings"), RawValues, /*bMaintainOrder=*/true);

    for (const FConfigValue& RawValue : RawValues)
    {
        const FString& Line = RawValue.GetValue();

        FInputAxisKeyMapping Mapping;
        const TCHAR* ImportPtr = *Line;
        if (FInputAxisKeyMapping::StaticStruct()->ImportText(
            ImportPtr, &Mapping, nullptr, PPF_None, GLog, TEXT("FInputAxisKeyMapping")))
        {
            Result.Add(Mapping);
        }
    }

    return Result;
}

void UBBRebindStuff::ResetAllMappingsToProjectDefaults()
{
    UInputSettings* Settings = UInputSettings::GetInputSettings();
    if (!Settings)
    {
        return;
    }

    const TArray<FInputActionKeyMapping> DefaultActions = GetDefaultActionMappings();
    const TArray<FInputAxisKeyMapping> DefaultAxes = GetDefaultAxisMappings();

    if (DefaultActions.Num() == 0 && DefaultAxes.Num() == 0)
    {
        UE_LOG(LogTemp, Error,
            TEXT("RebindHelper: Refusing to reset. GetDefaultActionMappings + GetDefaultAxisMappings both returned empty. ")
            TEXT("Check that DefaultInput.ini exists at the expected path and contains valid mappings."));
        return;
    }

#if WITH_EDITOR
    if (GIsEditor)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("RebindHelper: ResetAllMappingsToProjectDefaults called in editor. ")
            TEXT("Refusing to mutate UInputSettings (this would wipe Project Settings). ")
            TEXT("This function only does meaningful work in packaged builds. ")
            TEXT("If you need to reset bindings in editor for testing, delete Saved/Config/WindowsEditor/Input.ini and restart the editor."));
        return;
    }
#endif

    const TArray<FInputActionKeyMapping> CurrentActions = Settings->GetActionMappings();
    for (const FInputActionKeyMapping& M : CurrentActions)
    {
        Settings->RemoveActionMapping(M, /*bForceRebuildKeymaps=*/false);
    }

    const TArray<FInputAxisKeyMapping> CurrentAxes = Settings->GetAxisMappings();
    for (const FInputAxisKeyMapping& M : CurrentAxes)
    {
        Settings->RemoveAxisMapping(M, /*bForceRebuildKeymaps=*/false);
    }

    for (const FInputActionKeyMapping& M : DefaultActions)
    {
        Settings->AddActionMapping(M, /*bForceRebuildKeymaps=*/false);
    }
    for (const FInputAxisKeyMapping& M : DefaultAxes)
    {
        Settings->AddAxisMapping(M, /*bForceRebuildKeymaps=*/false);
    }

    Settings->ForceRebuildKeymaps();
    Settings->SaveKeyMappings();
}

bool UBBRebindStuff::IsGamepadKey(FKey Key)
{
    return Key.IsGamepadKey();
}

void UBBRebindStuff::RebindKey(FName ActionName, FKey NewKey, FKey OldKey)
{
    UInputSettings* Settings = UInputSettings::GetInputSettings();
    if (!Settings)
    {
        return;
    }

    if (OldKey.IsValid())
    {
        FInputActionKeyMapping OldMapping;
        OldMapping.ActionName = ActionName;
        OldMapping.Key = OldKey;
        Settings->RemoveActionMapping(OldMapping, /*bForceRebuildKeymaps=*/false);
    }

    FInputActionKeyMapping NewMapping;
    NewMapping.ActionName = ActionName;
    NewMapping.Key = NewKey;
    Settings->AddActionMapping(NewMapping, /*bForceRebuildKeymaps=*/false);

    Settings->ForceRebuildKeymaps();
    Settings->SaveKeyMappings();
}

void UBBRebindStuff::ClearActionMapping(FName ActionName, FKey KeyToClear)
{
    if (!KeyToClear.IsValid())
    {
        return;
    }

    UInputSettings* Settings = UInputSettings::GetInputSettings();
    if (!Settings)
    {
        return;
    }

    FInputActionKeyMapping Mapping;
    Mapping.ActionName = ActionName;
    Mapping.Key = KeyToClear;

    Settings->RemoveActionMapping(Mapping, /*bForceRebuildKeymaps=*/false);
    Settings->ForceRebuildKeymaps();
    Settings->SaveKeyMappings();
}

TArray<FInputActionKeyMapping> UBBRebindStuff::GetCurrentMappingsForAction(FName ActionName)
{
    TArray<FInputActionKeyMapping> Result;

    const UInputSettings* Settings = UInputSettings::GetInputSettings();
    if (!Settings)
    {
        return Result;
    }

    const TArray<FInputActionKeyMapping>& AllMappings = Settings->GetActionMappings();
    for (const FInputActionKeyMapping& Mapping : AllMappings)
    {
        if (Mapping.ActionName == ActionName)
        {
            Result.Add(Mapping);
        }
    }

    return Result;
}

void UBBRebindStuff::ResetActionMappingToDefault(FName ActionName)
{
    UInputSettings* Settings = UInputSettings::GetInputSettings();
    if (!Settings)
    {
        return;
    }

    const TArray<FInputActionKeyMapping> AllDefaults = GetDefaultActionMappings();
    TArray<FInputActionKeyMapping> ActionDefaults;
    for (const FInputActionKeyMapping& M : AllDefaults)
    {
        if (M.ActionName == ActionName)
        {
            ActionDefaults.Add(M);
        }
    }

    if (ActionDefaults.Num() == 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("RebindHelper: ResetActionMappingToDefault('%s') found no defaults for that action. ")
            TEXT("Refusing to wipe current bindings. Check ActionName spelling and DefaultInput.ini contents."),
            *ActionName.ToString());
        return;
    }

#if WITH_EDITOR
    if (GIsEditor)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("RebindHelper: ResetActionMappingToDefault called in editor. Refusing to mutate UInputSettings."));
        return;
    }
#endif

    const TArray<FInputActionKeyMapping> CurrentMappings = GetCurrentMappingsForAction(ActionName);
    for (const FInputActionKeyMapping& M : CurrentMappings)
    {
        Settings->RemoveActionMapping(M, /*bForceRebuildKeymaps=*/false);
    }

    for (const FInputActionKeyMapping& M : ActionDefaults)
    {
        Settings->AddActionMapping(M, /*bForceRebuildKeymaps=*/false);
    }

    Settings->ForceRebuildKeymaps();
    Settings->SaveKeyMappings();
}

// ============================================================================
// == GAMEPAD ICON LOOKUP IMPLEMENTATIONS (new) ==
// ============================================================================

const UDataTable* UBBRebindStuff::GetIconDataTable()
{
    // Cache the loaded table across calls. Static local is thread-safe in C++11+,
    // and this function is only called from the game thread in practice.
    static TWeakObjectPtr<UDataTable> CachedTable = nullptr;

    if (CachedTable.IsValid())
    {
        return CachedTable.Get();
    }

    UDataTable* Loaded = Cast<UDataTable>(GBBIconDataTablePath.TryLoad());
    if (!Loaded)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("BBKeyIcons: Failed to load icon DataTable at path '%s'. ")
            TEXT("Make sure the asset exists and the path matches GBBIconDataTablePath."),
            *GBBIconDataTablePath.ToString());
        return nullptr;
    }

    CachedTable = Loaded;
    return Loaded;
}

FName UBBRebindStuff::BuildRowName(FKey Key, EBBGamepadIconStyle Style)
{
    // Convert the enum to a plain string matching the CSV convention.
    // UEnum::GetValueAsString returns "EBBGamepadIconStyle::XboxOne" -> strip prefix.
    const FString StyleStr = UEnum::GetValueAsString(Style)
        .Replace(TEXT("EBBGamepadIconStyle::"), TEXT(""));

    // Row name format: "<Style>.<KeyName>" e.g. "XboxOne.Gamepad_FaceButton_Bottom"
    const FString RowNameStr = FString::Printf(TEXT("%s.%s"), *StyleStr, *Key.ToString());
    return FName(*RowNameStr);
}

UTexture2D* UBBRebindStuff::GetMissingIconTexture()
{
    // Same caching pattern as the DataTable - load once, reuse forever.
    static TWeakObjectPtr<UTexture2D> CachedMissing = nullptr;

    if (CachedMissing.IsValid())
    {
        return CachedMissing.Get();
    }

    UTexture2D* Loaded = Cast<UTexture2D>(GBBMissingIconTexturePath.TryLoad());
    if (!Loaded)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("BBKeyIcons: Failed to load missing-icon fallback at path '%s'. ")
            TEXT("Create a placeholder texture at this path � the rebind UI may show ")
            TEXT("nothing for unknown keys until then."),
            *GBBMissingIconTexturePath.ToString());
        return nullptr;
    }

    CachedMissing = Loaded;
    return Loaded;
}

TSoftObjectPtr<UTexture2D> UBBRebindStuff::GetGamepadIconSoftPtrForKey(FKey Key, EBBGamepadIconStyle Style)
{
    // Quick rejection: non-gamepad keys never have an icon in this system.
    if (!Key.IsGamepadKey())
    {
        return nullptr;
    }

    const UDataTable* IconTable = GetIconDataTable();
    if (!IconTable)
    {
        // DataTable failed to load - return null soft ptr.
        // Caller (GetGamepadIconForKey) will fall back to missing icon texture.
        return nullptr;
    }

    const FName RowName = BuildRowName(Key, Style);

    // FindRow with bWarnIfRowMissing=false so we don't spam logs when looking up
    // keys that legitimately aren't in the table (e.g. obscure gamepad keys).
    static const FString Context(TEXT("UBBRebindStuff::GetGamepadIconSoftPtrForKey"));
    const FBBKeyIconRow* Row = IconTable->FindRow<FBBKeyIconRow>(RowName, Context, /*bWarnIfRowMissing=*/false);
    if (!Row)
    {
        return nullptr;
    }

    return Row->Icon;
}

UTexture2D* UBBRebindStuff::GetGamepadIconForKey(FKey Key, EBBGamepadIconStyle Style)
{
    const TSoftObjectPtr<UTexture2D> SoftIcon = GetGamepadIconSoftPtrForKey(Key, Style);

    // If the soft pointer is null, fall back to missing-icon texture.
    if (SoftIcon.IsNull())
    {
        return GetMissingIconTexture();
    }

    // Force a synchronous load. For a rebind menu this is fine - the user is
    // sitting in a settings screen, no frame-time pressure.
    UTexture2D* Loaded = SoftIcon.LoadSynchronous();
    if (!Loaded)
    {
        // Soft pointer pointed somewhere, but the asset failed to load.
        // Likely a deleted/renamed texture the DataTable still references.
        UE_LOG(LogTemp, Warning,
            TEXT("BBKeyIcons: Row for key '%s' style %d has a broken Icon reference. ")
            TEXT("Check the DataTable row and re-assign the texture."),
            *Key.ToString(), (int32)Style);
        return GetMissingIconTexture();
    }

    return Loaded;
}
