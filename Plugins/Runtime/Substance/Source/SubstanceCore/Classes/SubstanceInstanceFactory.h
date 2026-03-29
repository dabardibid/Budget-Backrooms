// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceInstanceFactory.h

#pragma once
#include "substance/framework/typedefs.h"
#include "SubstanceInstanceFactory.generated.h"

/** Forward Declare */
class USubstanceGraphInstance;
class USubstanceInstanceFactory;
struct LegacyPackage;

namespace SubstanceAir
{
	struct GraphDesc;
	class PackageDesc;
}

/** User data referenced by the framework */
struct PackageDescData
{
	USubstanceInstanceFactory* ParentFactory;
};

UENUM(BlueprintType)
enum ESubstanceGenerationMode
{
	SGM_PlatformDefault = 0,
	SGM_Baked = 1,
	SGM_OnLoadSync = 2,
	SGM_OnLoadSyncAndCache = 3,
	SGM_OnLoadAsync = 4,
	SGM_OnLoadAsyncAndCache = 5,
	SGM_MAX = 6
};


USTRUCT(BlueprintType)
struct FSubstanceGraphDesc
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	int Index;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	FString Label;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	FString Description;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	FString Category;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	FString Keywords;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	FString Author;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	FString AuthorUrl;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Values")
	FString UserTag;
};



UCLASS(hideCategories = Object, BlueprintType)
class SUBSTANCECORE_API USubstanceInstanceFactory : public UObject
{
	GENERATED_UCLASS_BODY()

private:
	/** Stores references to the instances that are created with this factory */
	UPROPERTY(DuplicateTransient)
	TArray<USubstanceGraphInstance*> mGraphInstances;

	TArray<FSubstanceGraphDesc> mGraphDescs;

public:
	/** Framework container that stores all of the description data for owned graphs */
	SubstanceAir::shared_ptr<SubstanceAir::PackageDesc> SubstancePackage;

	/** Data references stored within the framework */
	PackageDescData mPackageUserData;

	/** Used for reimporting sbs files within editor */
	UPROPERTY()
	FString	RelativeSourceFilePath;

	/** Used for reimporting sbs files within editor */
	UPROPERTY()
	FString	AbsoluteSourceFilePath;

	/** Used to store when the file was created */
	UPROPERTY()
	FString	SourceFileTimestamp;

	/** Flag for the format used to save asset data */
	UPROPERTY()
	TEnumAsByte<ESubstanceGenerationMode> GenerationMode;

	/** Serializes the assets that were created and saved post framework refactor */
	void SerializeCurrent(FArchive& Ar);

	/** Serializes assets that were created before the framework refactor */
	void SerializeLegacy(FArchive& Ar);

	/** Begin UObject interface */
	virtual void Serialize(FArchive& Ar) override;
	virtual void BeginDestroy() override;
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	virtual void PostLoad() override;
	/** End UObject interface */

	/** Returns the resource size based on how we are saving the data */
	virtual void GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize) override;

	/** Increases the count of graph instances relying on this factory - Used for cleanup */
	void RegisterGraphInstance(USubstanceGraphInstance* Graph);

	/** Initialize this factory with a Substance Package */
	void Initialize(SubstanceAir::shared_ptr<SubstanceAir::PackageDesc> Package);

	/** Decreases the count of graph instances relying on this factory - Used for cleanup */
	void UnregisterGraphInstance(USubstanceGraphInstance* Graph);

	/** Returns the array of current associated graphs */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	const TArray<FSubstanceGraphDesc> GetGraphDescs();

	/** Returns the array of current associated graphs */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	const TArray<USubstanceGraphInstance*> GetGraphInstances();

	/** Returns a list of strings of all of the graph packages from the current graph instances */
	void ClearReferencingObjects();

	/** Creates a graph instance from a graph description */
	UFUNCTION(BlueprintCallable, Category = "Substance", meta = (DisplayName = "Create Graph Instance from Desc"))
	USubstanceGraphInstance* CreateGraphInstance(FSubstanceGraphDesc GraphDesc, FString PackageName);

};
