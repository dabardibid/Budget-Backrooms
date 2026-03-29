// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: AssetTypeActions_SubstanceInstanceFactory.h

#pragma once
#include "AssetTypeActions_Base.h"
#include "AssetTypeCategories.h"

class FAssetTypeActions_SubstanceInstanceFactory : public FAssetTypeActions_Base
{
public:
	/** Gets the Asset Actions Name */
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SubstanceInstanceFactory", "Substance Instance Factory"); }

	/** Returns the icon color for this asset within the content explorer window */
	virtual FColor GetTypeColor() const override { return FColor(32, 192, 32); }

	/** Gets the asset class that this class will be effecting */
	virtual UClass* GetSupportedClass() const override { return USubstanceInstanceFactory::StaticClass(); }

	/** Checks to see if the supported asset has actions */
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }

	/** Gets the actions for the supported asset */
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;

	/** Opens the asset editor for Graph Instance Factory */
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

	/** Returns the Categories this asset belongs to (EAssetTypeCategories) */
	virtual uint32 GetCategories() override { return EAssetTypeCategories::MaterialsAndTextures; }

	/** Creates a Sub menu when you right click on an instance factory to create more instances of graphs */
	void CreateInstanceSubMenu(class FMenuBuilder& MenuBuilder, TArray<TWeakObjectPtr<USubstanceInstanceFactory>> InstanceFactory);

protected:
	/** Returns additional tooltip information for the specified asset, if it has any (otherwise return the null widget) */
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override
	{
		return FText::FromString(TEXT("Substance Instance Factory."));
	}

private:
	/** Handler for when Edit is selected */
	void ExecuteEdit(TArray<TWeakObjectPtr<USubstanceInstanceFactory>> Objects);

	/** Handler for when Reimport is selected */
	void ExecuteReimport(TArray<TWeakObjectPtr<USubstanceInstanceFactory>> Objects);

	/** Handler for initialization */
	void ExecuteInstantiate(const SubstanceAir::GraphDesc* Desc, USubstanceInstanceFactory* Factory);

	/** Handling child object deletions*/
	void ExecuteDeleteWithOutputs(TArray<TWeakObjectPtr<USubstanceInstanceFactory>> Objects);
};
