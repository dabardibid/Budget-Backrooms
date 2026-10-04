#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameFramework/SaveGame.h"
#include "UBBBPHelpers.generated.h" // must match THIS header's file name, and stay the LAST include

// =============================================================================================
// Everything for the Game.sav consolidation lives in this one header:
//   FBBLegacySlot          - one old save slot name (+ optional variable prefix)
//   UBBGameSave            - the merged save object (Saved/SaveGames/Game.sav)
//   UUBB_BlueprintHelpers  - your function library, now with the migration functions
//   UBBMigrateSavesAsync   - the "Migrate Saves" node with On Finished / On Failed pins
// =============================================================================================

/** One old save slot to look for. SlotName = file name without ".sav". */
USTRUCT(BlueprintType)
struct FBBLegacySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BB|Save")
	FString SlotName;

	/** Optional: prepended to the OLD variable names to find them in UBBGameSave (only needed on name clashes). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BB|Save")
	FString Prefix;
};

/**
 * One save object for everything that used to live in separate Settings_* / State_* slots (Game.sav).
 *
 * SaveGameToSlot writes all non-transient properties and does NOT check the SaveGame flag,
 * so a plain UPROPERTY is enough to get a variable written to disk.
 *
 * Give each variable the SAME NAME and SAME TYPE as in the old save class so the migration
 * can copy it. On a name clash between two old classes, rename one here and set the matching
 * Prefix on its FBBLegacySlot.
 */
UCLASS(BlueprintType)
class BUDGETBACKROOMS_API UBBGameSave : public USaveGame
{
	GENERATED_BODY()

public:
	/** 0 = brand-new object. Bump GCurrentVersion in UBBBPHelpers.cpp when the layout changes. */
	UPROPERTY()
	int32 SaveVersion = 0;

	// ---------------------------------------------------------------------------------------
	// EXAMPLES ONLY - replace with your real variables (copy names/types from the old classes)
	// ---------------------------------------------------------------------------------------

	UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
	float MasterVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Settings|Audio")
	float MusicVolume = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category = "State|Flashlight")
	bool bFlashlightOn = false;
};

/**
 *
 */
UCLASS()
class BUDGETBACKROOMS_API UUBB_BlueprintHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ---------------------------------------------------------------------------------------
	// Save consolidation  ->  Saved/SaveGames/Game.sav
	//
	// "OlderSlots" = names of even older generations, OLDEST FIRST. They are applied before the
	// built-in list of current slots (Settings_*, State_*, ...), and later slots win on clashes.
	// ---------------------------------------------------------------------------------------

	static FString GetConsolidatedSlotName();
	static int32 GetUserIndex();

	/** Loads Game.sav. Returns nullptr if it doesn't exist or isn't a UBBGameSave. */
	static UBBGameSave* LoadConsolidated();

	/** Builds a save object from every legacy slot that exists. Does NOT write to disk. */
	static UBBGameSave* BuildFromLegacySlots(const TArray<FBBLegacySlot>& OlderSlots);

	/** Synchronous: load Game.sav, or build + write it from the legacy slots. Never null. */
	UFUNCTION(BlueprintCallable, Category = "BB|Save")
	static UBBGameSave* LoadOrMigrate(const TArray<FBBLegacySlot>& OlderSlots);

	UFUNCTION(BlueprintCallable, Category = "BB|Save")
	static bool SaveConsolidated(UBBGameSave* Save);

	/** Deletes the legacy files, but only if Game.sav loads fine. Don't ship this call in the first release. */
	UFUNCTION(BlueprintCallable, Category = "BB|Save")
	static void DeleteLegacySlots(const TArray<FBBLegacySlot>& OlderSlots);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBBMigrationResult, UBBGameSave*, Save);

/**
 * Blueprint node "Migrate Saves" with two exec outputs:
 *   On Finished (Save) - Game.sav is loaded, or was built from the legacy slots and written to disk
 *   On Failed   (Save) - the write didn't go through; Save is still a usable in-memory object, legacy files are untouched
 *
 * Use it in an Event Graph (e.g. GameInstance Init), not inside a function.
 * All the logic lives in UUBB_BlueprintHelpers; this class only exists because
 * exec pins need their own UBlueprintAsyncActionBase subclass.
 */
UCLASS()
class BUDGETBACKROOMS_API UBBMigrateSavesAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** OlderSlots: names of older save generations, OLDEST FIRST. The current Settings_* / State_* slots are built in. */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "BB|Save")
	static UBBMigrateSavesAsync* MigrateSaves(UObject* WorldContextObject, const TArray<FBBLegacySlot>& OlderSlots);

	UPROPERTY(BlueprintAssignable)
	FBBMigrationResult OnFinished;

	UPROPERTY(BlueprintAssignable)
	FBBMigrationResult OnFailed;

	virtual void Activate() override;

private:
	void HandleSaved(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	UPROPERTY()
	TArray<FBBLegacySlot> PendingOlderSlots;

	/** Keeps the object alive (GC) while the async write is running. */
	UPROPERTY()
	UBBGameSave* PendingSave = nullptr;
};
