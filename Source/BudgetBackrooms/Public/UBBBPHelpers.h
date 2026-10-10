// UBBBPHelpers.h
// Merges the old per-feature save slots into Settings.sav and Game.sav (all slots are BB_SaveSys objects),
// exposed as the async Blueprint node "Migrate Saves". Also has a reset helper, WipeAllSaves.
//
//   Settings_*                    -> Settings.sav
//   State_* / OlderSlots entries  -> Game.sav
//   AudioMaster, AudioMusic, BetterCallSaul, DiscordRPC, EarlyMPAcc, Flashlight, LVStatus,
//   Sensitivity, ViewSkVal        -> discarded (reset to defaults)

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameFramework/SaveGame.h"
#include "UBBBPHelpers.generated.h"

class FProperty;

enum class EBBSlotTarget : uint8
{
	Game,     // Game.sav
	Settings, // Settings.sav
	Discard   // not migrated
};

// SlotName is the file name without ".sav"
USTRUCT(BlueprintType)
struct FBBLegacySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget Savings")
	FString SlotName;

	// Optional prefix for the old variable names, for name clashes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget Savings")
	FString Prefix;
};

// Merge state, advanced one legacy slot at a time. Not reflected: the owner must keep Save / SettingsSave alive.
struct FBBMigrationBuilder
{
	TSubclassOf<USaveGame> SaveClass;

	USaveGame* Save = nullptr;
	USaveGame* SettingsSave = nullptr;

	bool bBuildGame = true;
	bool bBuildSettings = true;

	TArray<FBBLegacySlot> Slots;
	int32 NextIndex = 0;
	int32 SlotsRead = 0;
	bool bListValues = false;
	bool bDryRun = false;
	bool bDeleteOldFiles = false;

	double StartSeconds = 0.0;
	double WorkSeconds = 0.0;

	// Variables that already received a non-default value from an earlier slot
	TSet<const FProperty*> Assigned;
	TSet<const FProperty*> AssignedSettings;

	TArray<FString> Report;
};

UCLASS()
class BUDGETBACKROOMS_API UUBB_BlueprintHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// OlderSlots: extra legacy slot names, oldest first. SaveClass: the Blueprint save class (BB_SaveSys).

	static FString GetConsolidatedSlotName(); // "Game"
	static FString GetSettingsSlotName();     // "Settings"
	static int32 GetUserIndex();

	static USaveGame* LoadConsolidated(TSubclassOf<USaveGame> SaveClass);

	static USaveGame* LoadSettingsSlot(TSubclassOf<USaveGame> SaveClass);

	static bool IsAllDefaults(USaveGame* Save);

	static bool HasLegacyFiles(const TArray<FBBLegacySlot>& OlderSlots, EBBSlotTarget Target);

	// A file needs building if it is missing, or all-default while old files for it still exist
	// (e.g. the game created it empty before the migration ran). A file with real data is never touched.
	static bool NeedsMigration(TSubclassOf<USaveGame> SaveClass, const TArray<FBBLegacySlot>& OlderSlots,
		USaveGame*& OutGame, USaveGame*& OutSettings, bool& bOutBuildGame, bool& bOutBuildSettings, FString& OutReason);

	static int32 DeleteLegacyFilesNow(const TArray<FBBLegacySlot>& OlderSlots, TArray<FString>& Report);

	static void BeginBuild(FBBMigrationBuilder& B, TSubclassOf<USaveGame> SaveClass, const TArray<FBBLegacySlot>& OlderSlots,
		bool bListValues, bool bDryRun, bool bDeleteOldFiles = false, bool bBuildGame = true, bool bBuildSettings = true);

	// Processes one slot; returns true while more remain
	static bool StepBuild(FBBMigrationBuilder& B, FString& OutSlotName);

	static void FinishBuild(FBBMigrationBuilder& B);

	// Logs the report; also shown on screen in the editor
	static void PrintReportToScreen(const TArray<FString>& Report, float Duration = 30.f);

	// True if any legacy save file exists (built-in list + OlderSlots, discarded ones included)
	UFUNCTION(BlueprintPure, Category = "Budget Savings")
	static bool HasLegacySaves(const TArray<FBBLegacySlot>& OlderSlots);

	// Names of the legacy save files that exist on disk
	UFUNCTION(BlueprintCallable, Category = "Budget Savings")
	static TArray<FString> GetLegacySaveNames(const TArray<FBBLegacySlot>& OlderSlots);

	// True if there are legacy files and Migrate Saves would build Game.sav and/or Settings.sav from them
	UFUNCTION(BlueprintCallable, Category = "Budget Savings")
	static bool IsMigrationNeeded(TSubclassOf<USaveGame> SaveClass, const TArray<FBBLegacySlot>& OlderSlots);

	// Deletes only the discarded legacy files (never merged anyway). State_* / Settings_* files stay, they may still hold unmigrated data.
	// bDryRun only reports. Returns the number deleted
	UFUNCTION(BlueprintCallable, Category = "Budget Savings")
	static int32 DeleteLegacySaves(const TArray<FBBLegacySlot>& OlderSlots, bool bDryRun = false);

	// Destructive: deletes all legacy save files (and Game.sav / Settings.sav if bAlsoDeleteGameSav) without checks
	// bDryRun only reports. Returns the number of files deleted
	UFUNCTION(BlueprintCallable, Category = "Budget Savings")
	static int32 WipeAllSaves(const TArray<FBBLegacySlot>& OlderSlots, bool bAlsoDeleteGameSav = true, bool bDryRun = false);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBBMigrationResult, USaveGame*, Save, USaveGame*, Settings, const FString&, Report);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBBMigrationProgress, float, Progress, const FString&, CurrentSlot);

// "Migrate Saves" node (Save Class = BB_SaveSys). Reads one legacy slot per tick so a progress bar can update.
//   On Finished: Game.sav / Settings.sav are in use (already present, or built, written and read back)
//   On Failed:   a write failed or could not be verified; old files are untouched
//   bDryRun: report only, nothing written or deleted
//   bDeleteOldFiles: delete the old files after a verified write (never when nothing was merged)
//   StepDelay: seconds between slots
UCLASS()
class BUDGETBACKROOMS_API UBBMigrateSavesAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "Budget Savings")
	static UBBMigrateSavesAsync* MigrateSaves(UObject* WorldContextObject, TSubclassOf<USaveGame> SaveClass, const TArray<FBBLegacySlot>& OlderSlots, bool bDryRun = false, bool bDeleteOldFiles = false, float StepDelay = 0.0f);

	UPROPERTY(BlueprintAssignable)
	FBBMigrationProgress OnProgress;

	UPROPERTY(BlueprintAssignable)
	FBBMigrationResult OnFinished;

	UPROPERTY(BlueprintAssignable)
	FBBMigrationResult OnFailed;

	virtual void Activate() override;
	virtual void BeginDestroy() override;

private:
	bool TickStep(float DeltaTime);
	void WriteNext();
	void Finalize();
	void HandleSaved(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	UPROPERTY()
	TSubclassOf<USaveGame> PendingSaveClass;

	UPROPERTY()
	TArray<FBBLegacySlot> PendingOlderSlots;

	bool bPendingDryRun = false;
	bool bPendingDeleteOld = false;
	float PendingStepDelay = 0.0f;

	bool bWritingGame = false;
	bool bGameWritten = false;
	bool bSettingsWritten = false;
	bool bWriteFailed = false;

	FString PendingReport;
	FBBMigrationBuilder Builder;
	FDelegateHandle TickHandle;

	// Kept as UPROPERTYs so GC doesn't collect them mid-merge
	UPROPERTY()
	USaveGame* PendingSave = nullptr;

	UPROPERTY()
	USaveGame* PendingSettings = nullptr;
};