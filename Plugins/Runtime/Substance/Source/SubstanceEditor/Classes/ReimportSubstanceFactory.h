// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: ReimportSubstanceFactory.h

#pragma once
#include "SubstanceFactory.h"
#include "EditorReimportHandler.h"
#include "ReimportSubstanceFactory.generated.h"

//NOTE:: Function definitions located in SubstanceFactory.cpp
UCLASS(hideCategories = Object)
class UReimportSubstanceFactory : public USubstanceFactory, public FReimportHandler
{
public:
	GENERATED_UCLASS_BODY()

	/** FReimportHandler interface - checks to see if this asset can be reimported */
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;

	/** FReimportHandler interface - Sets the path to the file to reimport from */
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;

	/** FReimportHandler interface - Reimports the Obj */
	virtual EReimportResult::Type Reimport(UObject* Obj) override;

private:

	/** Saves all of the informations we need to recreate this factory */
	void SaveRecreationData(class USubstanceInstanceFactory* FactoryToReimport);
};
