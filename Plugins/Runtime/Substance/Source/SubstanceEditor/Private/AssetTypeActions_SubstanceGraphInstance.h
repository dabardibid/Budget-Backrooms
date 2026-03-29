// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: AssetTypeActions_SubstanceGraphInstance.h

#pragma once
#include "AssetTypeActions_Base.h"
#include "SubstanceGraphInstance.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

class FAssetTypeActions_SubstanceGraphInstance : public FAssetTypeActions_Base
{
public:
	/** Returns the name for this asset actions */
	virtual FText GetName() const override
	{
		return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SubstanceGraphInstance", "Substance Graph Instance");
	}

	/** Returns the icon color for this asset within the content explorer window */
	virtual FColor GetTypeColor() const override
	{
		return FColor(230, 30, 50);
	}

	/** Gets the asset class that this class will be effecting */
	virtual UClass* GetSupportedClass() const override
	{
		return USubstanceGraphInstance::StaticClass();
	}

	/** Checks to see if the supported asset has actions */
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override
	{
		return true;
	}

	/** Gets the actions for the supported asset */
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;

	/** Opens the asset editor for Graph Instances */
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

	/** Returns the Categories this asset belongs to */
	virtual uint32 GetCategories() override
	{
		return EAssetTypeCategories::MaterialsAndTextures;
	}

	/** Create dynamic sub menu with available presets */
	void CreatePresetsSubMenu(class FMenuBuilder& MenuBuilder, TArray<TWeakObjectPtr<USubstanceGraphInstance>> Graphs);

protected:
	/** Returns additional tooltip information for the specified asset, if it has any (otherwise return the null widget) */
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override
	{
		USubstanceGraphInstance* GraphInstance = Cast<USubstanceGraphInstance>(AssetData.GetAsset());

		if (GraphInstance && GraphInstance->ParentFactory)
		{
			return FText::FromString(TEXT("Substance Graph Instance."));
		}
		else
		{
			return FText::FromString(TEXT("Orphan Graph Instance, Instance Factory missing."));
		}
	}

private:
	/** Handler for when Edit is selected */
	void ExecuteEdit(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects);

	/** Handler for when Reimport is selected */
	void ExecuteReimport(TArray<TWeakObjectPtr<USubstanceInstanceFactory>> Objects);

	/** Handling sbsprs import */
	void ExecuteImportPresets(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects);

	/** Handling sbsprs export */
	void ExecuteExportPresets(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects);

	/** Handling sbsprs export */
	void ExecuteResetDefault(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects);

	/** Handling parent factory locating*/
	void ExecuteFindParentFactory(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects);

	/** Handling child objects deletion*/
	void ExecuteDeleteWithOutputs(TArray<TWeakObjectPtr<USubstanceGraphInstance>> Objects);
};
