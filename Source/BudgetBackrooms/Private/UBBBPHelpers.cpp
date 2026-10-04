#include "UBBBPHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY_STATIC(LogBBSaveMigration, Log, All);

namespace
{
	// New file: Saved/SaveGames/Game.sav
	const TCHAR* const GNewSlot = TEXT("Game");
	constexpr int32 GUserIndex = 0;
	constexpr int32 GCurrentVersion = 1;

	// Current generation, names as seen in Saved/SaveGames. Applied AFTER the OlderSlots
	// array, so these win on same-named variables.
	const TCHAR* const GCurrentGenSlots[] =
	{
		TEXT("BudgetBackrooms_Achievements"),
		TEXT("Settings_Audio"),
		TEXT("Settings_AudioMusic"),
		TEXT("Settings_DOF"),
		TEXT("Settings_Language"),
		TEXT("Settings_Other"),
		TEXT("Settings_RPC"),
		TEXT("Settings_ShakeInt"),
		TEXT("State_AccMP"),
		TEXT("State_AchVars"),
		TEXT("State_Classic"),
		TEXT("State_Flashlight"),
		TEXT("State_Interaction"),
		TEXT("State_PlayerVar"),
		TEXT("ViewSkVal"), // dated 2023 - check it's still needed
	};

	/** OlderSlots (oldest first) followed by the built-in current-generation slots. */
	TArray<FBBLegacySlot> BuildSlotList(const TArray<FBBLegacySlot>& OlderSlots)
	{
		TArray<FBBLegacySlot> All = OlderSlots;
		for (const TCHAR* Name : GCurrentGenSlots)
		{
			FBBLegacySlot Slot;
			Slot.SlotName = Name;
			All.Add(Slot);
		}
		return All;
	}

	/** Copies every same-named, same-typed property from Src into Dst. Returns how many were copied. */
	int32 CopyMatchingProperties(UObject* Src, UObject* Dst, const FString& Prefix)
	{
		int32 Copied = 0;

		for (TFieldIterator<FProperty> It(Src->GetClass()); It; ++It)
		{
			FProperty* SrcProp = *It;

			if (SrcProp->HasAnyPropertyFlags(CPF_Transient))
			{
				continue; // never written to disk, nothing to migrate
			}

			const FName DstName(*(Prefix + SrcProp->GetName()));
			FProperty* DstProp = FindFProperty<FProperty>(Dst->GetClass(), DstName);

			if (!DstProp)
			{
				UE_LOG(LogBBSaveMigration, Warning, TEXT("  no destination for %s.%s (dropped)"),
					*Src->GetClass()->GetName(), *SrcProp->GetName());
				continue;
			}

			if (!SrcProp->SameType(DstProp))
			{
				UE_LOG(LogBBSaveMigration, Warning, TEXT("  type mismatch for %s.%s (dropped)"),
					*Src->GetClass()->GetName(), *SrcProp->GetName());
				continue;
			}

			DstProp->CopyCompleteValue(
				DstProp->ContainerPtrToValuePtr<void>(Dst),
				SrcProp->ContainerPtrToValuePtr<void>(Src));
			++Copied;
		}

		return Copied;
	}
}

// ---------------------------------------------------------------------------------------------
// UUBB_BlueprintHelpers
// ---------------------------------------------------------------------------------------------

FString UUBB_BlueprintHelpers::GetConsolidatedSlotName()
{
	return GNewSlot;
}

int32 UUBB_BlueprintHelpers::GetUserIndex()
{
	return GUserIndex;
}

UBBGameSave* UUBB_BlueprintHelpers::LoadConsolidated()
{
	if (!UGameplayStatics::DoesSaveGameExist(GNewSlot, GUserIndex))
	{
		return nullptr;
	}
	// Future layout changes: check ->SaveVersion here and upgrade.
	return Cast<UBBGameSave>(UGameplayStatics::LoadGameFromSlot(GNewSlot, GUserIndex));
}

UBBGameSave* UUBB_BlueprintHelpers::BuildFromLegacySlots(const TArray<FBBLegacySlot>& OlderSlots)
{
	UBBGameSave* NewSave = Cast<UBBGameSave>(UGameplayStatics::CreateSaveGameObject(UBBGameSave::StaticClass()));
	check(NewSave);

	int32 SlotsRead = 0;
	for (const FBBLegacySlot& Legacy : BuildSlotList(OlderSlots))
	{
		if (Legacy.SlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(Legacy.SlotName, GUserIndex))
		{
			continue;
		}

		USaveGame* Old = UGameplayStatics::LoadGameFromSlot(Legacy.SlotName, GUserIndex);
		if (!Old)
		{
			UE_LOG(LogBBSaveMigration, Warning, TEXT("Could not load legacy slot %s (is its class still in the project?)"), *Legacy.SlotName);
			continue;
		}

		const int32 Copied = CopyMatchingProperties(Old, NewSave, Legacy.Prefix);
		UE_LOG(LogBBSaveMigration, Log, TEXT("Migrated %s: %d properties"), *Legacy.SlotName, Copied);
		++SlotsRead;
	}

	NewSave->SaveVersion = GCurrentVersion;
	UE_LOG(LogBBSaveMigration, Log, TEXT("Built consolidated save from %d legacy slots"), SlotsRead);
	return NewSave;
}

UBBGameSave* UUBB_BlueprintHelpers::LoadOrMigrate(const TArray<FBBLegacySlot>& OlderSlots)
{
	if (UBBGameSave* Existing = LoadConsolidated())
	{
		return Existing;
	}

	if (UGameplayStatics::DoesSaveGameExist(GNewSlot, GUserIndex))
	{
		// e.g. an older "Game" slot saved with a different class: list "Game" in OlderSlots so it gets merged.
		UE_LOG(LogBBSaveMigration, Warning, TEXT("%s exists but is not a UBBGameSave; rebuilding from legacy slots"), GNewSlot);
	}

	UBBGameSave* NewSave = BuildFromLegacySlots(OlderSlots);

	// Write it, then read it back before anyone is allowed to delete anything.
	if (!SaveConsolidated(NewSave) || !LoadConsolidated())
	{
		UE_LOG(LogBBSaveMigration, Error, TEXT("Writing %s failed; legacy files untouched, will retry next launch"), GNewSlot);
	}

	return NewSave;
}

bool UUBB_BlueprintHelpers::SaveConsolidated(UBBGameSave* Save)
{
	return Save && UGameplayStatics::SaveGameToSlot(Save, GNewSlot, GUserIndex);
}

void UUBB_BlueprintHelpers::DeleteLegacySlots(const TArray<FBBLegacySlot>& OlderSlots)
{
	// Safety net: never delete the old files unless the new one is readable.
	if (!LoadConsolidated())
	{
		UE_LOG(LogBBSaveMigration, Warning, TEXT("%s not loadable; refusing to delete legacy slots"), GNewSlot);
		return;
	}

	for (const FBBLegacySlot& Legacy : BuildSlotList(OlderSlots))
	{
		// Never delete the new file itself (an older "Game" slot may be listed).
		if (Legacy.SlotName.IsEmpty() || Legacy.SlotName == GNewSlot)
		{
			continue;
		}
		UGameplayStatics::DeleteGameInSlot(Legacy.SlotName, GUserIndex);
	}
}

// ---------------------------------------------------------------------------------------------
// UBBMigrateSavesAsync  ("Migrate Saves" node)
// ---------------------------------------------------------------------------------------------

UBBMigrateSavesAsync* UBBMigrateSavesAsync::MigrateSaves(UObject* WorldContextObject, const TArray<FBBLegacySlot>& OlderSlots)
{
	UBBMigrateSavesAsync* Action = NewObject<UBBMigrateSavesAsync>();
	Action->PendingOlderSlots = OlderSlots;

	// Keeps the action alive until SetReadyToDestroy() even if the calling Blueprint goes away.
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UBBMigrateSavesAsync::Activate()
{
	// Already migrated: nothing to do.
	if (UBBGameSave* Existing = UUBB_BlueprintHelpers::LoadConsolidated())
	{
		OnFinished.Broadcast(Existing);
		SetReadyToDestroy();
		return;
	}

	// First launch after the update: merge the legacy slots in memory, then write the result asynchronously.
	PendingSave = UUBB_BlueprintHelpers::BuildFromLegacySlots(PendingOlderSlots);

	UGameplayStatics::AsyncSaveGameToSlot(
		PendingSave,
		UUBB_BlueprintHelpers::GetConsolidatedSlotName(),
		UUBB_BlueprintHelpers::GetUserIndex(),
		FAsyncSaveGameToSlotDelegate::CreateUObject(this, &UBBMigrateSavesAsync::HandleSaved));
}

void UBBMigrateSavesAsync::HandleSaved(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	// Read it back so "finished" really means "it's on disk and loadable".
	const bool bOk = bSuccess && UUBB_BlueprintHelpers::LoadConsolidated() != nullptr;

	if (bOk)
	{
		OnFinished.Broadcast(PendingSave);
	}
	else
	{
		OnFailed.Broadcast(PendingSave);
	}

	PendingSave = nullptr;
	SetReadyToDestroy();
}
