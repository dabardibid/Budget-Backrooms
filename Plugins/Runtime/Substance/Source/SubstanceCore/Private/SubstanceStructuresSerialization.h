// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceStructuresSerialization

#pragma once
#include "substance/framework/typedefs.h"

//Forward Declares
class USubstanceGraphInstance;
class USubstanceInstanceFactory;
struct LegacyPackage;

namespace SubstanceAir
{
class PackageDesc;
};

/** FArchive Function Declarations*/
FArchive& operator<<(FArchive& Ar, SubstanceAir::shared_ptr<SubstanceAir::PackageDesc>& P);

char* LegacySerailizePackage(FArchive& Ar, USubstanceInstanceFactory* ParentFactory, int32& asmsize);
void LegacySerailizeGraphInstance(FArchive& Ar, USubstanceGraphInstance* Graph);
void FinalizeLegacyPresets(USubstanceGraphInstance* Graph);

//Legacy Serializing Data Structures
struct LegacyOutput
{
	FGuid LinkGuid;
	uint32 UID;
	int32 Format;
	int32 Width;
	int32 Height;
	uint32 MipsCount;
	int32 ChannelName;
	bool DynamicSize;
	FString Identifier;
	FString GuiLabel;
	FString GuiGroup;

	LegacyOutput()
	{
		UID = 0;
		Format = 0;
		Width = 512;
		Height = 512;
		MipsCount = 1;
		ChannelName = 0;
	}
};

struct LegacyInput
{
	uint32 UID;
	int32 Type;
	int32 Clamped;
	int32 GuiWidgetType;

	TArray<uint32> AlteringOutputs;

	FString Identifier;
	FString GuiLabel;

	FString GuiGroup;
	//FString GuiDescrition;

	TArray<float> DefaultValues;
	TArray<float> min;
	TArray<float> max;
	//float step;

	TMap<int32, FString> ComboboxValues;

	LegacyInput()
	{
		UID = 0;
		Type = 0;
		Clamped = 0;
		GuiWidgetType = 0;
	}
};

struct LegacyGuiGroup
{
	FString Identifier;
	FString Label;
};

struct LegacyGraph
{
	TArray<LegacyOutput> Outputs;
	TArray<LegacyInput> Inputs;
	TArray<LegacyGuiGroup> GuiGroups;
	TArray<FString> KeyWords;

	FString PackageURL;
	FString Categroy;
	FString Description;
	FString Author;
	FString Label;
};

struct LegacyPackage
{
	uint32 UID;
	int32 CookerBuildID;
	TArray<LegacyGraph> Graphs;
	FString PackageURL;
	FString Label;
	FString Description;
	FString Category;
	FString Author;

	LegacyPackage()
	{
		UID = 0;
		CookerBuildID = 0;
	}

	~LegacyPackage()
	{
	}
};
