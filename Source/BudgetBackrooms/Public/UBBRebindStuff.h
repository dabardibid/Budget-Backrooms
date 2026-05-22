// UBBRebindStuff.h
// Blueprint helper for reading project default input mappings from DefaultInput.ini
// and applying them at runtime. Designed for UE 4.27.2.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/PlayerInput.h"   // FInputActionKeyMapping, FInputAxisKeyMapping
#include "UBBRebindStuff.generated.h"

UCLASS()
class BUDGETBACKROOMS_API UBBRebindStuff : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Reads action mappings directly from the project's DefaultInput.ini on disk.
     * This bypasses any runtime modifications the user has made via SaveKeyMappings(),
     * so it represents the true project defaults.
     *
     * Works in editor and packaged builds (DefaultInput.ini is staged into the
     * packaged Config folder unless explicitly excluded).
     */
    UFUNCTION(BlueprintCallable, Category = "Budget Rebind|Defaults")
    static TArray<FInputActionKeyMapping> GetDefaultActionMappings();

    /**
     * Same as above, for axis mappings (movement, look, etc).
     */
    UFUNCTION(BlueprintCallable, Category = "Budget Rebind|Defaults")
    static TArray<FInputAxisKeyMapping> GetDefaultAxisMappings();

    /**
     * Wipes all current action+axis mappings from UInputSettings and replaces them
     * with the values read from DefaultInput.ini. Also calls SaveKeyMappings()
     * and ForceRebuildKeymaps() so the change takes effect immediately.
     *
     * This is what you should call from your "Reset to Defaults" button.
     */
    UFUNCTION(BlueprintCallable, Category = "Budget Rebind|Defaults")
    static void ResetAllMappingsToProjectDefaults();

    /**
     * Returns true if the given FKey is a gamepad key (face buttons, sticks,
     * triggers, dpad). Useful for filtering which entries to show in
     * KB+M vs gamepad rebind lists.
     */
    UFUNCTION(BlueprintPure, Category = "Budget Rebind|Utility")
    static bool IsGamepadKey(FKey Key);

    /**
     * Rebinds an action to a new key.
     *
     * - If OldKey is valid (not EKeys::Invalid), removes the (ActionName, OldKey) mapping first.
     * - Adds the (ActionName, NewKey) mapping.
     * - Rebuilds keymaps once at the end.
     * - Persists to the user's Saved/Config Input.ini via SaveKeyMappings().
     *
     * Safe to call with OldKey = an "empty" FKey (EKeys::Invalid) — it will skip
     * the remove step and just add. Useful for first-time bindings on actions
     * that previously had no key assigned for that device family.
     */
    UFUNCTION(BlueprintCallable, Category = "Budget Rebind|Mapping")
    static void RebindKey(FName ActionName, FKey NewKey, FKey OldKey);
 
    /**
     * Removes any mapping where Action == ActionName AND Key == KeyToClear.
     * Rebuilds keymaps and saves. Use for the per-slot "clear" button.
     *
     * If you want to clear an entire action regardless of device, call this
     * twice with the action's KBM key and gamepad key.
     */
    UFUNCTION(BlueprintCallable, Category = "Budget Rebind|Mapping")
    static void ClearActionMapping(FName ActionName, FKey KeyToClear);

    /**
    * Returns the current (live, post-rebind) action mappings filtered by ActionName.
    * Reads directly from UInputSettings, so it reflects any in-session rebinds.
    *
    * Useful for populating a rebind entry widget — pass self.ActionName and you'll
    * get back the 1-2 mappings (one KBM, one gamepad typically) for that action.
    * Returns an empty array if no mappings exist for that action.
    */
    UFUNCTION(BlueprintCallable, Category = "Budget Rebind|Mapping")
    static TArray<FInputActionKeyMapping> GetCurrentMappingsForAction(FName ActionName);

private:
    /**
     * Resolves the absolute path to the project's DefaultInput.ini.
     * Used internally so we read from the right file in both editor and packaged builds.
     */
    static FString GetDefaultInputIniPath();
};
