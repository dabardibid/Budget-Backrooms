// UBBBPHelpers.cpp

#include "UBBBPHelpers.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogBBSaveMigration, Log, All);

namespace
{
	const TCHAR* const GNewSlot = TEXT("Game");
	const TCHAR* const GSettingsSlot = TEXT("Settings");
	constexpr int32 GUserIndex = 0;

	// Legacy slot names, applied after OlderSlots (GDiscardSlots are appended too, see BuildSlotList)
	const TCHAR* const GCurrentGenSlots[] =
	{
		TEXT("DiscordRPC"),
		TEXT("EarlyMPAcc"),
		TEXT("Flashlight"),
		TEXT("LVStatus"),
		TEXT("Settings_DoF"),
		TEXT("Settings_Audio"),
		TEXT("Settings_AudioMusic"),
		TEXT("Settings_DOF"),
		TEXT("Settings_Language"),
		TEXT("Settings_Other"),
		TEXT("Settings_RPC"),
		TEXT("Settings_ShakeInt"),
		TEXT("Settings_Sensitivity"),
		TEXT("State_Classic"),
		TEXT("State_Flashlight"),
		TEXT("State_Interaction"),
		TEXT("State_PlayerVar"),
	};

	// Not migrated (reset to defaults), but still removed by "delete old files"
	const TCHAR* const GDiscardSlots[] =
	{
		TEXT("BudgetBackrooms_Achievements"),
		TEXT("State_AccMP"),
		TEXT("State_AchVars"),
		TEXT("AudioMaster"),
		TEXT("AudioMusic"),
		TEXT("BetterCallSaul"),
		TEXT("DiscordRPC"),
		TEXT("EarlyMPAcc"),
		TEXT("Flashlight"),
		TEXT("LVStatus"),
		TEXT("Sensitivity"),
		TEXT("ViewSkVal"),
	};

	struct FScopedWorkTimer
	{
		explicit FScopedWorkTimer(double& InTarget) : Target(InTarget), Start(FPlatformTime::Seconds()) {}
		~FScopedWorkTimer() { Target += FPlatformTime::Seconds() - Start; }
		double& Target;
		double Start;
	};

	bool IsReservedName(const FString& Name)
	{
		return Name.Equals(GNewSlot, ESearchCase::IgnoreCase) || Name.Equals(GSettingsSlot, ESearchCase::IgnoreCase);
	}

	// Settings_* -> Settings.sav, discard list -> dropped, everything else (State_*, OlderSlots) -> Game.sav
	EBBSlotTarget ClassifySlot(const FString& Name)
	{
		for (const TCHAR* Discard : GDiscardSlots)
		{
			if (Name.Equals(Discard, ESearchCase::IgnoreCase))
			{
				return EBBSlotTarget::Discard;
			}
		}
		if (Name.StartsWith(TEXT("Settings_"), ESearchCase::IgnoreCase))
		{
			return EBBSlotTarget::Settings;
		}
		return EBBSlotTarget::Game;
	}

	TArray<FBBLegacySlot> BuildSlotList(const TArray<FBBLegacySlot>& OlderSlots)
	{
		TArray<FBBLegacySlot> All = OlderSlots;
		for (const TCHAR* Name : GCurrentGenSlots)
		{
			FBBLegacySlot Slot;
			Slot.SlotName = Name;
			All.Add(Slot);
		}
		for (const TCHAR* Name : GDiscardSlots)
		{
			const bool bListed = All.ContainsByPredicate([Name](const FBBLegacySlot& S) { return S.SlotName.Equals(Name, ESearchCase::IgnoreCase); });
			if (!bListed)
			{
				FBBLegacySlot Slot;
				Slot.SlotName = Name;
				All.Add(Slot);
			}
		}
		return All;
	}

	TArray<FString> ExistingLegacyNames(const TArray<FBBLegacySlot>& AllSlots, const EBBSlotTarget* OnlyTarget = nullptr)
	{
		TArray<FString> Names;
		for (const FBBLegacySlot& Slot : AllSlots)
		{
			if (Slot.SlotName.IsEmpty() || IsReservedName(Slot.SlotName))
			{
				continue;
			}
			if (OnlyTarget && ClassifySlot(Slot.SlotName) != *OnlyTarget)
			{
				continue;
			}
			if (UGameplayStatics::DoesSaveGameExist(Slot.SlotName, GUserIndex))
			{
				Names.AddUnique(Slot.SlotName); // case-insensitive: Settings_DoF / Settings_DOF collapse
			}
		}
		return Names;
	}

	// The file must load back, and must not be empty if real data was merged
	bool VerifyWritten(bool bGameFile, TSubclassOf<USaveGame> SaveClass, USaveGame* Built)
	{
		USaveGame* Reloaded = bGameFile ? UUBB_BlueprintHelpers::LoadConsolidated(SaveClass) : UUBB_BlueprintHelpers::LoadSettingsSlot(SaveClass);
		return Reloaded && (UUBB_BlueprintHelpers::IsAllDefaults(Built) || !UUBB_BlueprintHelpers::IsAllDefaults(Reloaded));
	}

	// Blueprint names like "Level 5" -> "Level5", used as a fallback match
	FString ToCppIdentifier(const FString& In)
	{
		FString Out;
		for (const TCHAR C : In)
		{
			if (FChar::IsAlnum(C) || C == TEXT('_'))
			{
				Out.AppendChar(C);
			}
		}
		if (Out.IsEmpty() || FChar::IsDigit(Out[0]))
		{
			Out.InsertAt(0, TEXT('_'));
		}
		return Out;
	}

	struct FCopyResult
	{
		int32 Copied = 0;
		int32 SkippedDefault = 0;
		int32 Kept = 0;
	};

	// Variables that differ from the class default
	void CollectNonDefault(UObject* Obj, TSet<const FProperty*>& Out)
	{
		UObject* Defaults = Obj->GetClass()->GetDefaultObject();
		for (TFieldIterator<FProperty> It(Obj->GetClass()); It; ++It)
		{
			FProperty* Prop = *It;
			if (!Prop->HasAnyPropertyFlags(CPF_Transient)
				&& !Prop->Identical(Prop->ContainerPtrToValuePtr<void>(Obj), Prop->ContainerPtrToValuePtr<void>(Defaults), 0))
			{
				Out.Add(Prop);
			}
		}
	}

	// Copies same-named, same-typed properties from Src to Dst. Every old slot contains every variable of the
	// class, so a default value never overwrites one that an earlier slot already set.
	// Protected: variables the destination file already holds real data for, never overwritten.
	FCopyResult CopyMatchingProperties(UObject* Src, UObject* Dst, const FString& Prefix, const FString& SlotName,
		TSet<const FProperty*>& Assigned, const TSet<const FProperty*>& Protected, TArray<FString>* Report, bool bListValues)
	{
		FCopyResult Result;
		UObject* SrcDefaults = Src->GetClass()->GetDefaultObject();

		for (TFieldIterator<FProperty> It(Src->GetClass()); It; ++It)
		{
			FProperty* SrcProp = *It;

			if (SrcProp->HasAnyPropertyFlags(CPF_Transient))
			{
				continue;
			}

			const bool bIsDefault = SrcProp->Identical(
				SrcProp->ContainerPtrToValuePtr<void>(Src),
				SrcProp->ContainerPtrToValuePtr<void>(SrcDefaults),
				0);

			FProperty* DstProp = FindFProperty<FProperty>(Dst->GetClass(), FName(*(Prefix + SrcProp->GetName())));
			if (!DstProp)
			{
				DstProp = FindFProperty<FProperty>(Dst->GetClass(), FName(*(Prefix + ToCppIdentifier(SrcProp->GetName()))));
			}

			if (!DstProp)
			{
				UE_LOG(LogBBSaveMigration, Log, TEXT("  no destination for %s.%s%s"),
					*Src->GetClass()->GetName(), *SrcProp->GetName(), bIsDefault ? TEXT(" (default value)") : TEXT(" (NON-DEFAULT value, would be lost)"));
				if (Report && !bIsDefault)
				{
					Report->Add(FString::Printf(TEXT("[WARN] %s.%s: changed value, no variable \"%s\" in the new save class (would be lost)"),
						*SlotName, *SrcProp->GetName(), *(Prefix + SrcProp->GetName())));
				}
				continue;
			}

			if (!SrcProp->SameType(DstProp))
			{
				if (Report && !bIsDefault)
				{
					Report->Add(FString::Printf(TEXT("[WARN] %s.%s: changed value, but type differs from %s (would be lost)"),
						*SlotName, *SrcProp->GetName(), *DstProp->GetName()));
				}
				continue;
			}

			if (Protected.Contains(DstProp))
			{
				++Result.Kept;
				continue;
			}

			if (bIsDefault && Assigned.Contains(DstProp))
			{
				++Result.SkippedDefault;
				continue;
			}

			DstProp->CopyCompleteValue(
				DstProp->ContainerPtrToValuePtr<void>(Dst),
				SrcProp->ContainerPtrToValuePtr<void>(Src));
			Assigned.Add(DstProp);
			++Result.Copied;

			if (Report && bListValues)
			{
				FString ValueStr;
				DstProp->ExportTextItem(ValueStr, DstProp->ContainerPtrToValuePtr<void>(Dst), nullptr, Dst, PPF_None, nullptr);
				Report->Add(FString::Printf(TEXT("[VAL] %s.%s = %s%s"), *SlotName, *SrcProp->GetName(), *ValueStr.Left(120),
					bIsDefault ? TEXT("  (default)") : TEXT("")));
			}
		}

		return Result;
	}
}

FString UUBB_BlueprintHelpers::GetConsolidatedSlotName()
{
	return GNewSlot;
}

FString UUBB_BlueprintHelpers::GetSettingsSlotName()
{
	return GSettingsSlot;
}

int32 UUBB_BlueprintHelpers::GetUserIndex()
{
	return GUserIndex;
}

USaveGame* UUBB_BlueprintHelpers::LoadConsolidated(TSubclassOf<USaveGame> SaveClass)
{
	if (!SaveClass || !UGameplayStatics::DoesSaveGameExist(GNewSlot, GUserIndex))
	{
		return nullptr;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(GNewSlot, GUserIndex);
	if (Loaded && !Loaded->IsA(SaveClass))
	{
		UE_LOG(LogBBSaveMigration, Warning, TEXT("%s exists but is a %s, not a %s"), GNewSlot, *Loaded->GetClass()->GetName(), *SaveClass->GetName());
		return nullptr;
	}
	return Loaded;
}

USaveGame* UUBB_BlueprintHelpers::LoadSettingsSlot(TSubclassOf<USaveGame> SaveClass)
{
	if (!SaveClass || !UGameplayStatics::DoesSaveGameExist(GSettingsSlot, GUserIndex))
	{
		return nullptr;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(GSettingsSlot, GUserIndex);
	if (Loaded && !Loaded->IsA(SaveClass))
	{
		UE_LOG(LogBBSaveMigration, Warning, TEXT("%s exists but is a %s, not a %s"), GSettingsSlot, *Loaded->GetClass()->GetName(), *SaveClass->GetName());
		return nullptr;
	}
	return Loaded;
}

bool UUBB_BlueprintHelpers::IsAllDefaults(USaveGame* Save)
{
	if (!Save)
	{
		return true;
	}

	UObject* Defaults = Save->GetClass()->GetDefaultObject();
	for (TFieldIterator<FProperty> It(Save->GetClass()); It; ++It)
	{
		FProperty* Prop = *It;
		if (Prop->HasAnyPropertyFlags(CPF_Transient))
		{
			continue;
		}
		if (!Prop->Identical(Prop->ContainerPtrToValuePtr<void>(Save), Prop->ContainerPtrToValuePtr<void>(Defaults), 0))
		{
			return false;
		}
	}
	return true;
}

bool UUBB_BlueprintHelpers::HasLegacyFiles(const TArray<FBBLegacySlot>& OlderSlots, EBBSlotTarget Target)
{
	return ExistingLegacyNames(BuildSlotList(OlderSlots), &Target).Num() > 0;
}

bool UUBB_BlueprintHelpers::NeedsMigration(TSubclassOf<USaveGame> SaveClass, const TArray<FBBLegacySlot>& OlderSlots,
	USaveGame*& OutGame, USaveGame*& OutSettings, bool& bOutBuildGame, bool& bOutBuildSettings, FString& OutReason)
{
	OutGame = LoadConsolidated(SaveClass);
	OutSettings = LoadSettingsSlot(SaveClass);

	const bool bGameLegacy = HasLegacyFiles(OlderSlots, EBBSlotTarget::Game);
	const bool bSettingsLegacy = HasLegacyFiles(OlderSlots, EBBSlotTarget::Settings);
	const bool bDiscardLegacy = HasLegacyFiles(OlderSlots, EBBSlotTarget::Discard);

	bOutBuildGame = !OutGame || bGameLegacy;
	bOutBuildSettings = !OutSettings || bSettingsLegacy;

	TArray<FString> Why;

	if (!OutGame)
	{
		Why.Add(FString::Printf(TEXT("%s.sav does not exist (or is not a %s)"), GNewSlot, SaveClass ? *SaveClass->GetName() : TEXT("?")));
	}
	else if (bGameLegacy)
	{
		Why.Add(FString::Printf(TEXT("old save files found for %s.sav (existing values are kept)"), GNewSlot));
	}

	if (!OutSettings)
	{
		Why.Add(FString::Printf(TEXT("%s.sav does not exist (or is not a %s)"), GSettingsSlot, SaveClass ? *SaveClass->GetName() : TEXT("?")));
	}
	else if (bSettingsLegacy)
	{
		Why.Add(FString::Printf(TEXT("old Settings_ files found for %s.sav (existing values are kept)"), GSettingsSlot));
	}

	if (bDiscardLegacy)
	{
		Why.Add(TEXT("discarded old files found (not migrated, only removed)"));
	}

	if (Why.Num() == 0)
	{
		OutReason = FString::Printf(TEXT("%s.sav and %s.sav exist and no old save files are left"), GNewSlot, GSettingsSlot);
		return false;
	}

	OutReason = FString::Join(Why, TEXT("; "));
	return true;
}

int32 UUBB_BlueprintHelpers::DeleteLegacyFilesNow(const TArray<FBBLegacySlot>& OlderSlots, TArray<FString>& Report)
{
	int32 Deleted = 0;
	for (const FString& Name : ExistingLegacyNames(BuildSlotList(OlderSlots)))
	{
		if (UGameplayStatics::DeleteGameInSlot(Name, GUserIndex))
		{
			Report.Add(FString::Printf(TEXT("[OK]  %s: old file deleted"), *Name));
			++Deleted;
		}
		else
		{
			Report.Add(FString::Printf(TEXT("[ERR] %s: delete failed"), *Name));
			UE_LOG(LogBBSaveMigration, Warning, TEXT("Could not delete old save file %s"), *Name);
		}
	}
	Report.Add(FString::Printf(TEXT("Deleted %d old save file(s)"), Deleted));
	return Deleted;
}

void UUBB_BlueprintHelpers::BeginBuild(FBBMigrationBuilder& B, TSubclassOf<USaveGame> SaveClass, const TArray<FBBLegacySlot>& OlderSlots,
	bool bListValues, bool bDryRun, bool bDeleteOldFiles, bool bBuildGame, bool bBuildSettings)
{
	B = FBBMigrationBuilder();
	B.StartSeconds = FPlatformTime::Seconds();
	B.SaveClass = SaveClass;
	B.bListValues = bListValues;
	B.bDryRun = bDryRun;
	B.bDeleteOldFiles = bDeleteOldFiles;
	B.bBuildGame = bBuildGame;
	B.bBuildSettings = bBuildSettings;
	B.Slots = BuildSlotList(OlderSlots);

	{
		FScopedWorkTimer Timer(B.WorkSeconds);
		B.Save = SaveClass ? UGameplayStatics::CreateSaveGameObject(SaveClass) : nullptr;
		B.SettingsSave = SaveClass ? UGameplayStatics::CreateSaveGameObject(SaveClass) : nullptr;
	}

	if (!B.Save || !B.SettingsSave)
	{
		B.Report.Add(TEXT("[ERR] No save class given (set Save Class to BB_SaveSys)"));
		B.NextIndex = B.Slots.Num(); // nothing to step through
		return;
	}

	if (bDryRun)
	{
		// A dry run previews both files, whatever a normal run would decide
		B.bBuildGame = true;
		B.bBuildSettings = true;

		USaveGame* G = nullptr;
		USaveGame* S = nullptr;
		bool bBG = false;
		bool bBS = false;
		FString Reason;
		const bool bNeeds = NeedsMigration(SaveClass, OlderSlots, G, S, bBG, bBS, Reason);

		B.Report.Add(TEXT("[DRY RUN] nothing will be written or deleted"));
		if (bNeeds)
		{
			B.Report.Add(FString::Printf(TEXT("[DRY RUN] %s"), *Reason));
			B.Report.Add(FString::Printf(TEXT("[DRY RUN] a normal run would build: %s%s%s"),
				bBG ? GNewSlot : TEXT(""), (bBG && bBS) ? TEXT(" + ") : TEXT(""), bBS ? GSettingsSlot : TEXT("")));
		}
		else
		{
			B.Report.Add(FString::Printf(TEXT("[DRY RUN] %s - a normal run would just load them and skip all of this (this is only a preview)"), *Reason));
		}
	}

	// An existing file is the base: its values win, old slots only fill variables still at default
	if (B.bBuildGame)
	{
		if (USaveGame* Existing = LoadConsolidated(SaveClass))
		{
			B.Save = Existing;
			CollectNonDefault(Existing, B.Protected);
		}
	}
	if (B.bBuildSettings)
	{
		if (USaveGame* Existing = LoadSettingsSlot(SaveClass))
		{
			B.SettingsSave = Existing;
			CollectNonDefault(Existing, B.ProtectedSettings);
		}
	}
}

bool UUBB_BlueprintHelpers::StepBuild(FBBMigrationBuilder& B, FString& OutSlotName)
{
	if (!B.Save || !B.SettingsSave || !B.Slots.IsValidIndex(B.NextIndex))
	{
		return false;
	}

	FScopedWorkTimer Timer(B.WorkSeconds);

	const FBBLegacySlot& Legacy = B.Slots[B.NextIndex++];
	OutSlotName = Legacy.SlotName;

	if (!Legacy.SlotName.IsEmpty() && !IsReservedName(Legacy.SlotName))
	{
		const EBBSlotTarget Target = ClassifySlot(Legacy.SlotName);

		if (!UGameplayStatics::DoesSaveGameExist(Legacy.SlotName, GUserIndex))
		{
			B.Report.Add(FString::Printf(TEXT("[--]  %s: file not found"), *Legacy.SlotName));
		}
		else if (Target == EBBSlotTarget::Discard)
		{
			B.Report.Add(FString::Printf(TEXT("[SKIP] %s: old format, NOT migrated (reset to defaults)"), *Legacy.SlotName));
		}
		else if ((Target == EBBSlotTarget::Game && !B.bBuildGame) || (Target == EBBSlotTarget::Settings && !B.bBuildSettings))
		{
			B.Report.Add(FString::Printf(TEXT("[SKIP] %s: %s.sav already holds data, left alone"),
				*Legacy.SlotName, Target == EBBSlotTarget::Game ? GNewSlot : GSettingsSlot));
		}
		else
		{
			USaveGame* Old = UGameplayStatics::LoadGameFromSlot(Legacy.SlotName, GUserIndex);
			if (!Old)
			{
				B.Report.Add(FString::Printf(TEXT("[ERR] %s: file exists but failed to load (is its class still in the project?)"), *Legacy.SlotName));
			}
			else
			{
				const bool bToGame = (Target == EBBSlotTarget::Game);
				const FCopyResult Result = CopyMatchingProperties(Old, bToGame ? B.Save : B.SettingsSave, Legacy.Prefix, Legacy.SlotName,
					bToGame ? B.Assigned : B.AssignedSettings, bToGame ? B.Protected : B.ProtectedSettings, &B.Report, B.bListValues);
				UE_LOG(LogBBSaveMigration, Log, TEXT("Migrated %s -> %s.sav (%s): %d copied, %d skipped (still default), %d kept (already set)"),
					*Legacy.SlotName, bToGame ? GNewSlot : GSettingsSlot, *Old->GetClass()->GetName(), Result.Copied, Result.SkippedDefault, Result.Kept);
				B.Report.Add(FString::Printf(TEXT("[OK]  %s -> %s.sav (%s): %d copied, %d skipped (untouched default), %d kept (already set)"),
					*Legacy.SlotName, bToGame ? GNewSlot : GSettingsSlot, *Old->GetClass()->GetName(), Result.Copied, Result.SkippedDefault, Result.Kept));
				++B.SlotsRead;
			}
		}
	}

	return B.Slots.IsValidIndex(B.NextIndex);
}

void UUBB_BlueprintHelpers::FinishBuild(FBBMigrationBuilder& B)
{
	UE_LOG(LogBBSaveMigration, Log, TEXT("Built from %d legacy slots"), B.SlotsRead);
	B.Report.Add(FString::Printf(TEXT("Merged %d of %d listed slots"), B.SlotsRead, B.Slots.Num()));

	const FString TimingLine = FString::Printf(TEXT("Timing: %.1f ms of actual work, %.1f ms wall-clock (the rest is waiting between frames)"),
		B.WorkSeconds * 1000.0, (FPlatformTime::Seconds() - B.StartSeconds) * 1000.0);
	B.Report.Add(TimingLine);

	if (B.bDryRun)
	{
		if (B.bDeleteOldFiles)
		{
			const TArray<FString> Names = ExistingLegacyNames(B.Slots);
			B.Report.Add(FString::Printf(TEXT("[DRY RUN] after a successful write, %d old file(s) would be deleted: %s"),
				Names.Num(), *FString::Join(Names, TEXT(", "))));
		}

		B.Report.Add(TEXT("[DRY RUN] done"));
		PrintReportToScreen(B.Report);
	}
	else
	{
		UE_LOG(LogBBSaveMigration, Display, TEXT("%s"), *TimingLine);
	}
}

void UUBB_BlueprintHelpers::PrintReportToScreen(const TArray<FString>& Report, float Duration)
{
	for (const FString& Line : Report)
	{
		UE_LOG(LogBBSaveMigration, Display, TEXT("%s"), *Line);
	}

#if WITH_EDITOR
	if (!GEngine)
	{
		return;
	}

	// Newest on-screen message appears on top, so add in reverse to read top-to-bottom.
	for (int32 i = Report.Num() - 1; i >= 0; --i)
	{
		const FString& Line = Report[i];
		if (Line.StartsWith(TEXT("[VAL]")))
		{
			continue;
		}

		FColor Color = FColor::White;
		if (Line.StartsWith(TEXT("[ERR]"))) { Color = FColor::Red; }
		else if (Line.StartsWith(TEXT("[WARN]"))) { Color = FColor::Yellow; }
		else if (Line.StartsWith(TEXT("[SKIP]"))) { Color = FColor::Orange; }
		else if (Line.StartsWith(TEXT("[OK]"))) { Color = FColor::Green; }
		else if (Line.StartsWith(TEXT("[--]"))) { Color = FColor(150, 150, 150); }

		GEngine->AddOnScreenDebugMessage(INDEX_NONE, Duration, Color, Line);
	}
#endif
}

int32 UUBB_BlueprintHelpers::WipeAllSaves(const TArray<FBBLegacySlot>& OlderSlots, bool bAlsoDeleteGameSav, bool bDryRun)
{
	TArray<FString> Names;
	for (const FBBLegacySlot& Legacy : BuildSlotList(OlderSlots))
	{
		if (!Legacy.SlotName.IsEmpty())
		{
			Names.AddUnique(Legacy.SlotName); // case-insensitive: Settings_DoF / Settings_DOF collapse
		}
	}

	if (bAlsoDeleteGameSav)
	{
		Names.AddUnique(FString(GNewSlot));
		Names.AddUnique(FString(GSettingsSlot));
	}
	else
	{
		Names.Remove(FString(GNewSlot));
		Names.Remove(FString(GSettingsSlot));
	}

	TArray<FString> Report;
	if (bDryRun)
	{
		Report.Add(TEXT("[DRY RUN] nothing will be deleted"));
	}

	int32 Deleted = 0;
	for (const FString& Name : Names)
	{
		if (!UGameplayStatics::DoesSaveGameExist(Name, GUserIndex))
		{
			Report.Add(FString::Printf(TEXT("[--]  %s: file not found"), *Name));
			continue;
		}

		if (bDryRun)
		{
			Report.Add(FString::Printf(TEXT("[OK]  %s: would be deleted"), *Name));
			++Deleted;
		}
		else if (UGameplayStatics::DeleteGameInSlot(Name, GUserIndex))
		{
			Report.Add(FString::Printf(TEXT("[OK]  %s: deleted"), *Name));
			++Deleted;
		}
		else
		{
			Report.Add(FString::Printf(TEXT("[ERR] %s: delete failed"), *Name));
		}
	}

	Report.Add(FString::Printf(TEXT("%s %d save file(s)"), bDryRun ? TEXT("Would delete") : TEXT("Deleted"), Deleted));
	PrintReportToScreen(Report);
	return Deleted;
}

bool UUBB_BlueprintHelpers::HasLegacySaves(const TArray<FBBLegacySlot>& OlderSlots)
{
	return ExistingLegacyNames(BuildSlotList(OlderSlots)).Num() > 0;
}

TArray<FString> UUBB_BlueprintHelpers::GetLegacySaveNames(const TArray<FBBLegacySlot>& OlderSlots)
{
	return ExistingLegacyNames(BuildSlotList(OlderSlots));
}

bool UUBB_BlueprintHelpers::IsMigrationNeeded(TSubclassOf<USaveGame> SaveClass, const TArray<FBBLegacySlot>& OlderSlots)
{
	USaveGame* Game = nullptr;
	USaveGame* Settings = nullptr;
	bool bBuildGame = false;
	bool bBuildSettings = false;
	FString Reason;

	const TArray<FString> Found = ExistingLegacyNames(BuildSlotList(OlderSlots));
	const bool bNeeded = Found.Num() > 0 && NeedsMigration(SaveClass, OlderSlots, Game, Settings, bBuildGame, bBuildSettings, Reason);

	UE_LOG(LogBBSaveMigration, Display, TEXT("Is Migration Needed: %s - %d old file(s) found: %s%s%s"),
		bNeeded ? TEXT("YES") : TEXT("NO"), Found.Num(), *FString::Join(Found, TEXT(", ")), Reason.IsEmpty() ? TEXT("") : TEXT(" | "), *Reason);
	return bNeeded;
}

int32 UUBB_BlueprintHelpers::DeleteLegacySaves(const TArray<FBBLegacySlot>& OlderSlots, bool bDryRun)
{
	const EBBSlotTarget Discard = EBBSlotTarget::Discard;
	TArray<FString> Report;
	int32 Count = 0;
	if (bDryRun)
	{
		Report.Add(TEXT("[DRY RUN] nothing will be deleted"));
	}

	for (const FString& Name : ExistingLegacyNames(BuildSlotList(OlderSlots), &Discard))
	{
		if (bDryRun)
		{
			Report.Add(FString::Printf(TEXT("[OK]  %s: would be deleted"), *Name));
			++Count;
		}
		else if (UGameplayStatics::DeleteGameInSlot(Name, GUserIndex))
		{
			Report.Add(FString::Printf(TEXT("[OK]  %s: deleted"), *Name));
			++Count;
		}
		else
		{
			Report.Add(FString::Printf(TEXT("[ERR] %s: delete failed"), *Name));
		}
	}

	Report.Add(FString::Printf(TEXT("%s %d old save file(s)"), bDryRun ? TEXT("Would delete") : TEXT("Deleted"), Count));
	PrintReportToScreen(Report);
	return Count;
}

UBBMigrateSavesAsync* UBBMigrateSavesAsync::MigrateSaves(UObject* WorldContextObject, TSubclassOf<USaveGame> SaveClass, const TArray<FBBLegacySlot>& OlderSlots, bool bDryRun, bool bDeleteOldFiles, float StepDelay)
{
	UBBMigrateSavesAsync* Action = NewObject<UBBMigrateSavesAsync>();
	Action->PendingSaveClass = SaveClass;
	Action->PendingOlderSlots = OlderSlots;
	Action->bPendingDryRun = bDryRun;
	Action->bPendingDeleteOld = bDeleteOldFiles;
	Action->PendingStepDelay = FMath::Max(0.f, StepDelay);
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UBBMigrateSavesAsync::Activate()
{
	if (!PendingSaveClass)
	{
		UE_LOG(LogBBSaveMigration, Error, TEXT("Migrate Saves: Save Class is not set (use BB_SaveSys)"));
		OnFailed.Broadcast(nullptr, nullptr, TEXT("[ERR] No save class given (set Save Class to BB_SaveSys)"));
		SetReadyToDestroy();
		return;
	}

	USaveGame* ExistingGame = nullptr;
	USaveGame* ExistingSettings = nullptr;
	bool bBuildGame = true;
	bool bBuildSettings = true;

	// Nothing to merge: finish right away without deleting anything (a dry run always previews)
	if (!bPendingDryRun)
	{
		FString Reason;
		if (!UUBB_BlueprintHelpers::NeedsMigration(PendingSaveClass, PendingOlderSlots, ExistingGame, ExistingSettings, bBuildGame, bBuildSettings, Reason))
		{
			UE_LOG(LogBBSaveMigration, Log, TEXT("Migrate Saves: nothing to do - %s"), *Reason);
			OnProgress.Broadcast(1.f, FString());
			OnFinished.Broadcast(ExistingGame, ExistingSettings, Reason);
			SetReadyToDestroy();
			return;
		}
		UE_LOG(LogBBSaveMigration, Log, TEXT("Migrate Saves: migrating - %s"), *Reason);
	}

	UUBB_BlueprintHelpers::BeginBuild(Builder, PendingSaveClass, PendingOlderSlots, bPendingDryRun, bPendingDryRun, bPendingDeleteOld, bBuildGame, bBuildSettings);

	// A file that is not rebuilt keeps its existing object
	PendingSave = Builder.bBuildGame ? Builder.Save : ExistingGame;
	PendingSettings = Builder.bBuildSettings ? Builder.SettingsSave : ExistingSettings;

	OnProgress.Broadcast(0.f, FString());

	TickHandle = FTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UBBMigrateSavesAsync::TickStep), PendingStepDelay);
}

bool UBBMigrateSavesAsync::TickStep(float DeltaTime)
{
	FString SlotName;
	const bool bMore = UUBB_BlueprintHelpers::StepBuild(Builder, SlotName);

	const float Total = static_cast<float>(FMath::Max(1, Builder.Slots.Num()));
	OnProgress.Broadcast(FMath::Clamp(static_cast<float>(Builder.NextIndex) / Total, 0.f, 1.f), SlotName);

	if (bMore)
	{
		return true;
	}

	TickHandle.Reset(); // returning false below removes the ticker
	UUBB_BlueprintHelpers::FinishBuild(Builder);
	PendingReport = FString::Join(Builder.Report, TEXT("\n"));

	if (!Builder.Save || !Builder.SettingsSave)
	{
		OnFailed.Broadcast(nullptr, nullptr, PendingReport);
		SetReadyToDestroy();
		return false;
	}

	if (bPendingDryRun)
	{
		OnFinished.Broadcast(PendingSave, PendingSettings, PendingReport);
		PendingSave = nullptr;
		PendingSettings = nullptr;
		SetReadyToDestroy();
		return false;
	}

	WriteNext();
	return false;
}

void UBBMigrateSavesAsync::WriteNext()
{
	if (!bWriteFailed)
	{
		if (Builder.bBuildGame && !bGameWritten)
		{
			bWritingGame = true;
			UGameplayStatics::AsyncSaveGameToSlot(
				Builder.Save,
				UUBB_BlueprintHelpers::GetConsolidatedSlotName(),
				UUBB_BlueprintHelpers::GetUserIndex(),
				FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UBBMigrateSavesAsync::HandleSaved));
			return;
		}

		if (Builder.bBuildSettings && !bSettingsWritten)
		{
			bWritingGame = false;
			UGameplayStatics::AsyncSaveGameToSlot(
				Builder.SettingsSave,
				UUBB_BlueprintHelpers::GetSettingsSlotName(),
				UUBB_BlueprintHelpers::GetUserIndex(),
				FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UBBMigrateSavesAsync::HandleSaved));
			return;
		}
	}

	Finalize();
}

void UBBMigrateSavesAsync::HandleSaved(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	if (!bSuccess)
	{
		bWriteFailed = true;
	}
	else if (bWritingGame)
	{
		bGameWritten = true;
	}
	else
	{
		bSettingsWritten = true;
	}

	WriteNext();
}

void UBBMigrateSavesAsync::Finalize()
{
	// Verified: every written file loads again and, if real data was merged, is not empty
	bool bOk = !bWriteFailed;
	if (bOk && Builder.bBuildGame)
	{
		bOk = VerifyWritten(true, PendingSaveClass, Builder.Save);
	}
	if (bOk && Builder.bBuildSettings)
	{
		bOk = VerifyWritten(false, PendingSaveClass, Builder.SettingsSave);
	}

	if (bOk)
	{
		if (bPendingDeleteOld)
		{
			TArray<FString> DeleteReport;
			UUBB_BlueprintHelpers::DeleteLegacyFilesNow(PendingOlderSlots, DeleteReport);
			UUBB_BlueprintHelpers::PrintReportToScreen(DeleteReport);
			PendingReport += TEXT("\n") + FString::Join(DeleteReport, TEXT("\n"));
		}
		OnFinished.Broadcast(PendingSave, PendingSettings, PendingReport);
	}
	else
	{
		PendingReport += TEXT("\n[ERR] Writing Game.sav / Settings.sav failed or could not be verified - old files were NOT deleted");
		UE_LOG(LogBBSaveMigration, Error, TEXT("Save write failed or could not be verified; old files untouched"));
		OnFailed.Broadcast(PendingSave, PendingSettings, PendingReport);
	}

	PendingSave = nullptr;
	PendingSettings = nullptr;
	SetReadyToDestroy();
}

void UBBMigrateSavesAsync::BeginDestroy()
{
	if (TickHandle.IsValid())
	{
		FTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
	Super::BeginDestroy();
}