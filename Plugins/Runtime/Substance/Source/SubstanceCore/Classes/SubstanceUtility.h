// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceUtility.h

#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SubstanceUtility.generated.h"

UENUM(BlueprintType)
enum ESubstanceTextureSize
{
	ERL_16 UMETA(DisplayName = "16"),
	ERL_32 UMETA(DisplayName = "32"),
	ERL_64 UMETA(DisplayName = "64"),
	ERL_128 UMETA(DisplayName = "128"),
	ERL_256 UMETA(DisplayName = "256"),
	ERL_512 UMETA(DisplayName = "512"),
	ERL_1024 UMETA(DisplayName = "1024"),
	ERL_2048 UMETA(DisplayName = "2048"),
	ERL_4096 UMETA(DisplayName = "4096"),
	ERL_8192 UMETA(DisplayName = "8192")
};

USTRUCT(BlueprintType)
struct FSubstanceConnection
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Substance")
	FString OutputIdentifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Substance")
	FString InputImageIdentifier;
};

UCLASS(BlueprintType, MinimalAPI)
class USubstanceUtility : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/* Get the list of Substance Graph Instances used by a material */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	static SUBSTANCECORE_API TArray<class USubstanceGraphInstance*> GetSubstances(class UMaterialInterface* Material);

	/* Get the textures (from enabled outputs) of a Substance Graph Instance*/
	UFUNCTION(BlueprintCallable, Category = "Substance")
	static SUBSTANCECORE_API TArray<class UTexture2D*> GetSubstanceTextures(class USubstanceGraphInstance* GraphInstance);

	/* Get the original graph name */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	static SUBSTANCECORE_API FString GetGraphName(USubstanceGraphInstance* GraphInstance);

	/* Get the name of the factory that created this instance */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	static SUBSTANCECORE_API FString GetFactoryName(USubstanceGraphInstance* GraphInstance);

	/* Get the current rendering progress of the Substances currently loaded ([0, 1.0]) */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	static SUBSTANCECORE_API float GetSubstanceLoadingProgress();

	/* Create a dynamic Substance Graph Instance (no outputs enabled by default) */
	UFUNCTION(BlueprintCallable, Category = "Substance", meta = (WorldContext = "WorldContextObject"))
	static SUBSTANCECORE_API USubstanceGraphInstance * CreateGraphInstance(
	    UObject* WorldContextObject,
	    class USubstanceInstanceFactory* Factory,
	    int32 GraphDescIndex,
		UMaterial* ParentMaterial,
	    FString InstanceName = TEXT(""));

	/* Create a copy of Substance Graph Instance */
	UFUNCTION(BlueprintCallable, Category = "Substance", meta = (WorldContext = "WorldContextObject"))
	static SUBSTANCECORE_API USubstanceGraphInstance * DuplicateGraphInstance(UObject* WorldContextObject, USubstanceGraphInstance* GraphInstance);

	/* Create the textures of a Substance Graph Instance (enable its outputs) using the output indices */
	UFUNCTION(BlueprintCallable, Category = "Substance", meta = (WorldContext = "WorldContextObject"))
	static SUBSTANCECORE_API void EnableInstanceOutputs(UObject* WorldContextObject, USubstanceGraphInstance* GraphInstance, TArray<int32> OutputIndices);

	/* Disable the textures of a Substance Graph Instance (disable its outputs) using the output indices */
	UFUNCTION(BlueprintCallable, Category = "Substance", meta = (WorldContext = "WorldContextObject"))
	static SUBSTANCECORE_API void DisableInstanceOutputs(UObject* WorldContextObject, USubstanceGraphInstance* GraphInstance, TArray<int32> OutputIndices);

	/* Copy the inputs values from a Substance Graph Instance to another one */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	static SUBSTANCECORE_API void CopyInputParameters(USubstanceGraphInstance* SourceGraphInstance, USubstanceGraphInstance* DestGraphInstance);

	/* Reset the input values of a Substance Graph Instance to their default values */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	static SUBSTANCECORE_API void ResetInputParameters(USubstanceGraphInstance* GraphInstance);

	/* Set the output size input of the specified GraphInstance */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	static SUBSTANCECORE_API void SetGraphInstanceOutputSize(USubstanceGraphInstance* GraphInstance, ESubstanceTextureSize Width, ESubstanceTextureSize Height);

	/* Set the output size input of the specified GraphInstance */
	UFUNCTION(BlueprintCallable, Category = "Substance", meta = (DisplayName = "Set GraphInstance Output Size (Int)"))
	static SUBSTANCECORE_API void SetGraphInstanceOutputSizeInt(USubstanceGraphInstance* GraphInstance, int32 Width, int32 Height);

	/* Clear Substance Memory Cache */
	UFUNCTION(BlueprintCallable, Category = "Substance|Memory")
	static SUBSTANCECORE_API void ClearCache();

	/* Create an aggregate substance factory by fusing output images from one substance to the input images of another substance */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	static SUBSTANCECORE_API USubstanceInstanceFactory * CreateAggregateSubstanceFactory(
	    USubstanceInstanceFactory* OutputFactory,
	    int32 OutputFactoryGraphIndex,
	    USubstanceInstanceFactory* InputFactory,
	    int32 InputFactoryGraphIndex,
	    const TArray<FSubstanceConnection>& Connections
	);

	/* Queue a Substance Graph Instance in the renderer */
	UFUNCTION(BlueprintCallable, Category = "Substance|Render")
	static SUBSTANCECORE_API void AsyncRendering(USubstanceGraphInstance* InstancesToRender);

	/* Start the synchronous rendering of a Substance */
	UFUNCTION(BlueprintCallable, Category = "Substance|Render")
	static SUBSTANCECORE_API void SyncRendering(USubstanceGraphInstance* InstancesToRender);

	/** Returns an array of the material templates included with the substance plugin (may return none if the editor has not fully loaded yet) */
	UFUNCTION(BlueprintCallable, Category = "Substance")
	static SUBSTANCECORE_API TArray<UMaterial*> GetSubstanceIncludedMaterials();
};
