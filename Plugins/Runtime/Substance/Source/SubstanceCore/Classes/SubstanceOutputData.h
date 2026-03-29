// Copyright 2020 Allegorithmic Inc. All rights reserved.
// File: SubstanceOutputData.h

#pragma once

#include "Materials/MaterialLayersFunctions.h"

#include "substance/framework/typedefs.h"
#include "substance/framework/callbacks.h"
#include "SubstanceOutputData.generated.h"

class USubstanceGraphInstance;

/** Output structure for substance data to connect with material expressions*/
UCLASS(hideCategories = Object, BlueprintType)
class SUBSTANCECORE_API USubstanceOutputData : public UObject
{
	GENERATED_UCLASS_BODY()
	
	UPROPERTY()
	UObject* ConnectedObject;

public:
	UPROPERTY()
	FMaterialParameterInfo ParamInfo;

	UPROPERTY()
	USubstanceGraphInstance* ParentInstance;

	UPROPERTY()
	FGuid CacheGuid;

	UObject* GetData();

	void SetData(UObject* inData);

};

