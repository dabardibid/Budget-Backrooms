// Copyright 2020 Allegorithmic Inc. All rights reserved.
// File: SubstanceOutputData.cpp

#include "SubstanceOutputData.h"
#include "SubstanceCorePrivatePCH.h"
#include "SubstanceGraphInstance.h"

USubstanceOutputData::USubstanceOutputData(class FObjectInitializer const& PCIP) : Super(PCIP)
{
	ConnectedObject = nullptr;
}

UObject* USubstanceOutputData::GetData()
{
	return ConnectedObject;
}

void USubstanceOutputData::SetData(UObject* inData)
{
	ConnectedObject = inData;
}
