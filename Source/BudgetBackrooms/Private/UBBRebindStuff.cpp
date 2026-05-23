#include "UBBRebindStuff.h"
#include "GameFramework/InputSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "InputCoreTypes.h"

FString UBBRebindStuff::GetDefaultInputIniPath()
{
    // FPaths::SourceConfigDir() points to <Project>/Config in editor,
    // and to the staged Config folder in packaged builds. This is where
    // DefaultInput.ini lives — NOT the Saved/Config folder, which holds
    // user-modified overrides.
    return FPaths::SourceConfigDir() / TEXT("DefaultInput.ini");
}

TArray<FInputActionKeyMapping> UBBRebindStuff::GetDefaultActionMappings()
{
    TArray<FInputActionKeyMapping> Result;

    const FString IniPath = GetDefaultInputIniPath();

    // Load the ini file into a standalone FConfigFile so we read the on-disk
    // file directly, with no user overrides mixed in.
    FConfigFile ConfigFile;
    ConfigFile.Read(IniPath);

    if (ConfigFile.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("RebindHelper: Could not read DefaultInput.ini at %s"), *IniPath);
        return Result;
    }

    // Action and axis mappings live under [/Script/Engine.InputSettings].
    const FConfigSection* Section = ConfigFile.Find(TEXT("/Script/Engine.InputSettings"));
    if (!Section)
    {
        UE_LOG(LogTemp, Warning, TEXT("RebindHelper: No [/Script/Engine.InputSettings] section found"));
        return Result;
    }

    // Each "+ActionMappings=(...)" line is stored under the key "ActionMappings".
    // MultiFind returns all values for that key, in declaration order.
    TArray<FConfigValue> RawValues;
    Section->MultiFind(TEXT("ActionMappings"), RawValues, /*bMaintainOrder=*/true);

    for (const FConfigValue& RawValue : RawValues)
    {
        const FString& Line = RawValue.GetValue();

        // FInputActionKeyMapping is a UStruct; ImportText handles parsing the
        // "(ActionName=..., Key=..., bShift=..., ...)" string form natively.
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

    // Snapshot the defaults BEFORE we start mutating live settings.
    const TArray<FInputActionKeyMapping> DefaultActions = GetDefaultActionMappings();
    const TArray<FInputAxisKeyMapping> DefaultAxes = GetDefaultAxisMappings();

    // SAFETY GUARD 1: Refuse to wipe if defaults read returned empty.
    // Otherwise a failed DefaultInput.ini read would destroy all bindings with
    // nothing to restore. This protects against bad paths, missing files,
    // and parse failures.
    if (DefaultActions.Num() == 0 && DefaultAxes.Num() == 0)
    {
        UE_LOG(LogTemp, Error,
            TEXT("RebindHelper: Refusing to reset. GetDefaultActionMappings + GetDefaultAxisMappings both returned empty. ")
            TEXT("Check that DefaultInput.ini exists at the expected path and contains valid mappings."));
        return;
    }

    // SAFETY GUARD 2: In editor PIE, UInputSettings IS the project asset.
    // Mutating it would empty Project Settings → Engine → Input. Refuse.
    // In packaged builds GIsEditor is false, so the real reset proceeds.
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

    // Wipe current mappings. We copy the arrays first because Remove* mutates
    // the underlying list as we iterate.
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

    // Re-add defaults.
    for (const FInputActionKeyMapping& M : DefaultActions)
    {
        Settings->AddActionMapping(M, /*bForceRebuildKeymaps=*/false);
    }
    for (const FInputAxisKeyMapping& M : DefaultAxes)
    {
        Settings->AddAxisMapping(M, /*bForceRebuildKeymaps=*/false);
    }

    // One rebuild at the end, not per-mapping.
    Settings->ForceRebuildKeymaps();

    // Persist to the user's Saved/Config Input.ini so it survives a restart.
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

    // Skip remove step if OldKey isn't a real key (e.g. first-time binding).
    // EKeys::Invalid is the "empty" FKey used when nothing was previously bound.
    if (OldKey.IsValid())
    {
        FInputActionKeyMapping OldMapping;
        OldMapping.ActionName = ActionName;
        OldMapping.Key = OldKey;
        // Modifier flags (Shift/Ctrl/Alt/Cmd) default to false on a fresh struct,
        // which matches how Blueprint's Make InputActionKeyMapping behaves with
        // unchecked modifier pins.

        Settings->RemoveActionMapping(OldMapping, /*bForceRebuildKeymaps=*/false);
    }

    // Add the new mapping.
    FInputActionKeyMapping NewMapping;
    NewMapping.ActionName = ActionName;
    NewMapping.Key = NewKey;
    Settings->AddActionMapping(NewMapping, /*bForceRebuildKeymaps=*/false);

    // One rebuild, one save at the end.
    Settings->ForceRebuildKeymaps();
    Settings->SaveKeyMappings();
}

void UBBRebindStuff::ClearActionMapping(FName ActionName, FKey KeyToClear)
{
    if (!KeyToClear.IsValid())
    {
        return;  // Nothing to clear.
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

    // GetActionMappings() returns the full live array. We filter here so the
    // caller doesn't have to.
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

    // Read all defaults from disk, then filter to this action's defaults.
    const TArray<FInputActionKeyMapping> AllDefaults = GetDefaultActionMappings();
    TArray<FInputActionKeyMapping> ActionDefaults;
    for (const FInputActionKeyMapping& M : AllDefaults)
    {
        if (M.ActionName == ActionName)
        {
            ActionDefaults.Add(M);
        }
    }

    // SAFETY GUARD: if no defaults found for this action, abort.
    // Don't wipe a real action's bindings just because someone misspelled
    // the action name or DefaultInput.ini doesn't have it.
    if (ActionDefaults.Num() == 0)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("RebindHelper: ResetActionMappingToDefault('%s') found no defaults for that action. ")
            TEXT("Refusing to wipe current bindings. Check ActionName spelling and DefaultInput.ini contents."),
            *ActionName.ToString());
        return;
    }

    // Editor PIE guard, same reasoning as in ResetAllMappingsToProjectDefaults.
#if WITH_EDITOR
    if (GIsEditor)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("RebindHelper: ResetActionMappingToDefault called in editor. Refusing to mutate UInputSettings."));
        return;
    }
#endif

    // Remove all current mappings for this action.
    const TArray<FInputActionKeyMapping> CurrentMappings = GetCurrentMappingsForAction(ActionName);
    for (const FInputActionKeyMapping& M : CurrentMappings)
    {
        Settings->RemoveActionMapping(M, /*bForceRebuildKeymaps=*/false);
    }

    // Re-add the defaults for this action.
    for (const FInputActionKeyMapping& M : ActionDefaults)
    {
        Settings->AddActionMapping(M, /*bForceRebuildKeymaps=*/false);
    }

    Settings->ForceRebuildKeymaps();
    Settings->SaveKeyMappings();
}
