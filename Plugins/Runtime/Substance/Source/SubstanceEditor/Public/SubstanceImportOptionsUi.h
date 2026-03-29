// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceImportOptionsUi.h

#pragma once

#include "Materials/Material.h"

#include "SubstanceImportOptionsUi.generated.h"

UENUM(BlueprintType)		
enum class ESubstanceMaterialParentType : uint8
{
	Default = 0,
	Custom = 1,
	Generated = 2
};

UCLASS(config = EditorUserSettings, AutoExpandCategories = (General, Materials), HideCategories = Object)
class USubstanceImportOptionsUi : public UObject
{
	GENERATED_UCLASS_BODY()

	/** Use the string in "Name" field as base name of factory instance and textures */
	UPROPERTY(EditAnywhere, config, Category = General)
	uint32 bOverrideFullName : 1;

	/** Whether or not to override instance path */
	UPROPERTY(EditAnywhere, config, Category = General)
	uint32 bOverrideInstancePath : 1;

	/** Whether or not to override material path */
	UPROPERTY(EditAnywhere, config, Category = General)
	uint32 bOverrideMaterialPath : 1;

	/** Whether to automatically create graph instances for every graph description present in package*/
	UPROPERTY(EditAnywhere, config, Category = General)
	uint32 bCreateInstance : 1;

	/** Whether or not to force create an instance */
	uint32 bForceCreateInstance : 1;

	/** Whether to automatically create Unreal materials for instances */
	UPROPERTY(EditAnywhere, config, Category = Materials)
	uint32 bCreateMaterial : 1;

	/** Enables Material Selection button for custom templates on import*/
	UPROPERTY(EditAnywhere, config, Category = Materials)
	ESubstanceMaterialParentType uMaterialParentType;

	/** Parent Material used to create Unreal material instances */
	UPROPERTY(EditAnywhere, Category = Materials)
	UMaterial* ParentMaterial;

	/** Instance suggested name (based on filename) */
	UPROPERTY(EditAnywhere, config, Category = General)
	FString InstanceName;

	/** Instance suggested name (based on filename) */
	UPROPERTY(EditAnywhere, config, Category = General)
	FString MaterialName;

	/** Force the Graph Instance objects' path */
	UPROPERTY(EditAnywhere, config, Category = General)
	FString InstanceDestinationPath;

	/** Force the Texture Objects objects' path */
	UPROPERTY(EditAnywhere, config, Category = General)
	FString MaterialDestinationPath;
};
