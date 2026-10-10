// BBKeyIconTypes.h
// Types for the in-house gamepad key icon lookup system.
// Replaces the Xelu Icons plugin to avoid UE-171275 RHI init crash interaction.
//
// Designed for UE 4.27.2. Gamepad icons only � keyboard rebinds use plain text
// fallback in the rebind widget.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "BBKeyIconTypes.generated.h"

/**
 * Visual style for gamepad button icons. Matches Xelu's enum so the existing
 * CSV (with rows named "XboxOne.Gamepad_FaceButton_Bottom" etc.) imports cleanly.
 *
 * Naming convention for DataTable row names: "<StyleString>.<KeyName>"
 * e.g. "XboxOne.Gamepad_FaceButton_Bottom"
 *      "PS5.Gamepad_Special_Right"
 *      "Switch.Gamepad_LeftStick"
 *
 * The string form comes from UEnum::GetValueAsString() stripped of the enum prefix.
 * See UBBRebindStuff::BuildRowName() for the exact transformation.
 */
UENUM(BlueprintType)
enum class EBBGamepadIconStyle : uint8
{
    XboxOne     UMETA(DisplayName = "Xbox One"),
    XboxSeries  UMETA(DisplayName = "Xbox Series"),
    PS4         UMETA(DisplayName = "PlayStation 4"),
    PS5         UMETA(DisplayName = "PlayStation 5"),
};

/**
 * DataTable row struct for a single gamepad icon entry.
 * Matches Xelu's FXeluIconsInputsMetaData schema (one Icon field) so the CSV
 * imports without modification.
 *
 * Add fields here later if you need extras (display name, size hint, alt state).
 * Adding fields is non-breaking � re-importing the CSV will leave new fields
 * at their default values for existing rows.
 */
USTRUCT(BlueprintType)
struct BUDGETBACKROOMS_API FBBKeyIconRow : public FTableRowBase
{
    GENERATED_BODY()

    /** The texture to display for this key in this style. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BB Key Icon")
    TSoftObjectPtr<UTexture2D> Icon;
};
