// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceStructureSerialization.cpp

#include "SubstanceStructuresSerialization.h"
#include "SubstanceCorePrivatePCH.h"
#include "Engine/Texture2D.h"
#include "UObject/UObjectBaseUtility.h"
#include "SubstanceCoreCustomVersion.h"
#include "SubstanceGraphInstance.h"
#include "SubstanceCoreHelpers.h"
#include "SubstanceCoreCustomVersion.h"
#include "SubstanceInstanceFactory.h"
#include "substance/framework/package.h"
#include "substance/framework/preset.h"

/** FArchive Function Definitions*/
FArchive& operator<<(FArchive& Ar, SubstanceAir::shared_ptr<SubstanceAir::PackageDesc>& P)
{
	if (Ar.IsSaving())
	{
		//Write Assembly data blob
		uint32 asmSize = P->getAssemblyData()->length();
		uint32 xmlSize = P->getXmlString().length();

		Ar << asmSize;
		Ar << xmlSize;

		Ar.ByteOrderSerialize((void*)P->getAssemblyData()->c_str(), asmSize);
		Ar.ByteOrderSerialize((void*)P->getXmlString().c_str(), xmlSize);

		//Check to see if this is a duplicate writer - If so, save temp transfer data
		//NOTE::This is okay because this will never serialize for cooking and is only ran whenever and object is being duplicated
		if (Ar.GetArchiveName() == FString("FDuplicateDataWriter"))
		{
			Ar.ByteOrderSerialize((void*)&P->mUserData, sizeof(P->mUserData));
		}

	}
	else if (Ar.IsLoading())
	{
		//Read Assembly data blob
		uint32 asmSize, xmlSize;

		Ar << asmSize;
		Ar << xmlSize;

		char* asmData = new char[asmSize];
		char* xmlData = new char[xmlSize + 1];

		Ar.ByteOrderSerialize(asmData, asmSize);
		Ar.ByteOrderSerialize(xmlData, xmlSize);

		xmlData[xmlSize] = 0;

		P = SubstanceAir::make_shared<SubstanceAir::PackageDesc>(asmData, asmSize, xmlData);

		delete[] asmData;
		delete[] xmlData;

		//Check to see if this is a duplicate reader - If so, read temp transfer data
		//NOTE::This is okay because this will never serialize for cooking and is only ran whenever and object is being duplicated
		if (Ar.GetArchiveName() == FString("FDuplicateDataReader"))
		{
			Ar.ByteOrderSerialize((void*)&P->mUserData, sizeof(P->mUserData));
		}
	}

	return Ar;
}

FString LegacyChannelToString(int32 ChannelIndex)
{
	enum LegacyChannelUse
	{
		CHAN_Undef = 0,
		CHAN_Diffuse,
		CHAN_Opacity,
		CHAN_Emissive,
		CHAN_Ambient,
		CHAN_AmbientOcclusion,
		CHAN_Mask,
		CHAN_Normal,
		CHAN_Bump,
		CHAN_Height,
		CHAN_Displacement,
		CHAN_Specular,
		CHAN_SpecularLevel,
		CHAN_SpecularColor,
		CHAN_Glossiness,
		CHAN_Roughness,
		CHAN_AnisotropyLevel,
		CHAN_AnisotropyAngle,
		CHAN_Transmissive,
		CHAN_Reflection,
		CHAN_Refraction,
		CHAN_Environment,
		CHAN_IOR,
		CU_SCATTERING0,
		CU_SCATTERING1,
		CU_SCATTERING2,
		CU_SCATTERING3,
		CHAN_Metallic,
		CHAN_BaseColor,
		CHAN_MAX
	};

	switch ((LegacyChannelUse)ChannelIndex)
	{
	case CHAN_Diffuse:
		return FString("diffuse");
	case CHAN_BaseColor:
		return FString("baseColor");
	case CHAN_Metallic:
		return FString("metallic");
	case CHAN_SpecularColor:
		return FString("specularcolor");
	case CHAN_Specular:
		return FString("specular");
	case CHAN_Roughness:
		return FString("roughness");
	case CHAN_Emissive:
		return FString("emissive");
	case CHAN_Normal:
		return FString("normal");
	case CHAN_Displacement:
		return FString("displacement");
	case CHAN_Mask:
		return FString("mask");
	case CHAN_Opacity:
		return FString("opacity");
	case CHAN_Refraction:
		return FString("refraction");
	case CHAN_AmbientOcclusion:
		return FString("ambientOcclusion");
	default:
		return FString("");
	}
}

/** Legacy Serialization */
void LegacySerializeInputValue(FArchive& Ar, LegacyInput& Input, SubstanceAir::Preset::InputValue& CurPreset, USubstanceGraphInstance* Graph);

void LegacySerializeOutputInstance(FArchive& Ar, LegacyOutput& Output)
{
	int32 IsEnabled;

	Ar << Output.UID;
	Ar << Output.Format;
	Ar << Output.LinkGuid;
	Ar << IsEnabled;
}

void LegacySerializeArrayValues(FArchive& Ar, TArray<float>& Values, int32 SubstanceType)
{
	switch ((SubstanceIOType)SubstanceType)
	{
	case Substance_IOType_Float:
		{
			float Value;
			Ar << Value;
			Values.Push(Value);
		}
		break;
	case Substance_IOType_Float2:
		{
			SubstanceAir::Vec2Float Value;
			float CurValue;
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
		}
		break;
	case Substance_IOType_Float3:
		{
			SubstanceAir::Vec3Float Value;
			float CurValue;
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
		}
		break;
	case Substance_IOType_Float4:
		{
			SubstanceAir::Vec4Float Value;
			float CurValue;
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
		}
		break;
	case Substance_IOType_Integer:
		{
			int32 Value;
			Ar << Value;
			Values.Push(Value);
		}
		break;
	case Substance_IOType_Integer2:
		{
			SubstanceAir::Vec2Int Value;
			int32 CurValue;
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
		}
		break;
	case Substance_IOType_Integer3:
		{
			SubstanceAir::Vec3Int Value;
			int32 CurValue;
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
		}
		break;
	case Substance_IOType_Integer4:
		{
			SubstanceAir::Vec4Int Value;
			int32 CurValue;
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
			Ar << CurValue;
			Values.Push(CurValue);
		}
		break;
	case Substance_IOType_Image:
		{
			FString Label;
			FString Desc;

			Ar << Label;
			Ar << Desc;
		}
		break;
	}
}

void LegacySerializeInputDesc(FArchive& Ar, LegacyInput& Input)
{
	int32 type;
	Ar << type; //We were storing type twice?

	Ar << Input.Identifier;
	Ar << Input.GuiLabel;
	Ar << Input.UID;
	Ar << Input.Type;

	//Unused data
	uint32 BurnIndex = 0;
	int32 BurnHints = 0;
	int32 BurnHeavy = 0;

	Ar << BurnIndex; //Index

	Input.AlteringOutputs.BulkSerialize(Ar);

	Input.GuiWidgetType = 0;

	Ar << BurnHints; //UseHints
	Ar << BurnHeavy; //HeavyDuty
	Ar << Input.GuiWidgetType;

	if (Input.Type == Substance_IOType_Image)
	{
		LegacySerializeArrayValues(Ar, Input.DefaultValues, Input.Type);
	}
	else
	{
		if (Input.GuiWidgetType == SubstanceAir::InputWidget::Input_Combobox)
		{
			Ar << Input.ComboboxValues;
		}

		Ar << Input.Clamped;

		LegacySerializeArrayValues(Ar, Input.DefaultValues, Input.Type);
		LegacySerializeArrayValues(Ar, Input.min, Input.Type);
		LegacySerializeArrayValues(Ar, Input.max, Input.Type);

		Ar << Input.GuiGroup;
	}
}

void LegacySerializeInputInstance(FArchive& Ar, LegacyInput& Input, USubstanceGraphInstance* Graph)
{
	int32 HeavyDuty = 0;
	Ar << Input.UID;
	Ar << Input.Type;
	Ar << HeavyDuty;

	SubstanceAir::Preset::InputValue Value;
	Value.mType = (SubstanceIOType)Input.Type;
	Value.mUid = Input.UID;

	LegacySerializeInputValue(Ar, Input, Value, Graph);
	Graph->InstancePreset->mInputValues.push_back(Value);
}

void LegacySerializeInputImage(SubstanceAir::InputInstanceImage* Image)
{
}

void FinalizeLegacyPresets(USubstanceGraphInstance* Graph)
{
	std::vector<SubstanceAir::Preset::InputValue>::iterator PreItr = Graph->InstancePreset->mInputValues.begin();
	for (; PreItr != Graph->InstancePreset->mInputValues.end(); ++PreItr)
	{
		//Find the Graph that matches our UID and add in the last value element
		auto It = Graph->Instance->getInputs().cbegin();
		for (; It != Graph->Instance->getInputs().end(); ++It)
		{
			if ((*It)->mDesc.mUid == PreItr->mUid)
			{
				PreItr->mIdentifier = (*It)->mDesc.mIdentifier;
				break;
			}
		}
	}
}

void LegacySerailizeGraphInstance(FArchive& Ar, USubstanceGraphInstance* Graph)
{
	SubstanceAir::Preset LegacyPreset;

	//Input count
	int32 InputCount = 0;
	Ar << InputCount;

	//Input instance base
	for (int32 i = 0; i < InputCount; ++i)
	{
		//We already have this data - Serializing to stay synced
		LegacyInput Input;
		int32 Type;
		Ar << Type;

		LegacySerializeInputInstance(Ar, Input, Graph);
	}

	//Output count
	int32 OutputCount = 0;
	Ar << OutputCount;

	//Burn the array count that was saved by serializing an array
	int32 doublecount = 0;
	Ar << doublecount;

	//Output Instance
	for (int32 o = 0; o < OutputCount; ++o)
	{
		//We already have this data - Serializing to stay synced
		LegacyOutput Output;
		LegacySerializeOutputInstance(Ar, Output);
		Graph->OutputTextureLinkData.Add(Output.UID, Output.LinkGuid);
	}

	//No longer used
	FGuid InstanceGuid;
	Ar << InstanceGuid;

	FString PackageURL;
	Ar << PackageURL;
	Graph->PackageURL = PackageURL;
}

char* LegacySerailizePackage(FArchive& Ar, USubstanceInstanceFactory* ParentFactory, int32& asmsize)
{
	//Local Ref
	LegacyPackage pack;

	//Guid is unused data
	FGuid Guid;

	TArray<uint32> SubstanceUids;
	Ar << SubstanceUids;
	Ar << Guid;
	Ar << ParentFactory->AbsoluteSourceFilePath;
	Ar << ParentFactory->SourceFileTimestamp;

	//Package UID was never solely saved in previous versions. Using the first in the list of Substance UID's.
	pack.UID = SubstanceUids[0];

	//Serialize asm data
	int32 ElementSize = 0;
	Ar << ElementSize;
	Ar << asmsize;
	char* AsmData = new char[asmsize];
	Ar.ByteOrderSerialize(AsmData, asmsize);

	int32 GraphCount;
	Ar << GraphCount;

	//Loop through graphs and serialize them!
	for (int32 g = 0; g < GraphCount; g++)
	{
		LegacyGraph Graph;

		//Pulling in graph desc data
		Ar << Graph.PackageURL;
		Ar << Graph.Label;
		Ar << Graph.Description;

		//OutputDesc Array
		int32 OutputDescCount;
		Ar << OutputDescCount;

		for (int32 o = 0; o < OutputDescCount; ++o)
		{
			LegacyOutput Output;
			Ar << Output.Identifier;
			Ar << Output.GuiLabel;
			Ar << Output.UID;
			Ar << Output.Format;
			Ar << Output.ChannelName;

			TArray<uint32> BurnAlteringInputUID;
			BurnAlteringInputUID.BulkSerialize(Ar);

			Graph.Outputs.Push(Output);
		}

		//GUID - Instance ID Array
		int32 GuidCount = 0;
		Ar << GuidCount;

		FGuid BurnGUID;
		for (int32 gu = 0; gu < GuidCount; ++gu)
		{
			Ar << BurnGUID;
		}

		//Grab Input Descriptions
		int32 InputDescCount;
		Ar << InputDescCount;

		//Serialize inputs
		for (int32 i = 0; i < InputDescCount; ++i)
		{
			LegacyInput Input;
			LegacySerializeInputDesc(Ar, Input);
			Graph.Inputs.Push(Input);
		}

		pack.Graphs.Push(Graph);
	}

	return AsmData;
}

void LegacySerializeInputValue(FArchive& Ar, LegacyInput& Input, SubstanceAir::Preset::InputValue& PresetVal, USubstanceGraphInstance* Graph)
{
	switch ((SubstanceIOType)Input.Type)
	{
	case Substance_IOType_Float:
		{
			float Value;
			Ar << Value;

			FString ValueStr = FString::SanitizeFloat(Value);
			PresetVal.mValue = TCHAR_TO_ANSI(*ValueStr);
		}
		break;
	case Substance_IOType_Float2:
		{
			SubstanceAir::Vec2Float Value;
			float CurValue;
			Ar << CurValue;
			Value.x = CurValue;
			Ar << CurValue;
			Value.y = CurValue;

			FString ValueStr;
			ValueStr += FString::SanitizeFloat(Value.x);
			ValueStr += ",";
			ValueStr += FString::SanitizeFloat(Value.y);
			PresetVal.mValue = TCHAR_TO_ANSI(*ValueStr);
		}
		break;
	case Substance_IOType_Float3:
		{
			SubstanceAir::Vec3Float Value;
			float CurValue;
			Ar << CurValue;
			Value.x = CurValue;
			Ar << CurValue;
			Value.y = CurValue;
			Ar << CurValue;
			Value.z = CurValue;

			FString ValueStr;
			ValueStr += FString::SanitizeFloat(Value.x);
			ValueStr += ",";
			ValueStr += FString::SanitizeFloat(Value.y);
			ValueStr += ",";
			ValueStr += FString::SanitizeFloat(Value.z);
			PresetVal.mValue = TCHAR_TO_ANSI(*ValueStr);
		}
		break;
	case Substance_IOType_Float4:
		{
			SubstanceAir::Vec4Float Value;
			float CurValue;
			Ar << CurValue;
			Value.x = CurValue;
			Ar << CurValue;
			Value.y = CurValue;
			Ar << CurValue;
			Value.z = CurValue;
			Ar << CurValue;
			Value.w = CurValue;

			FString ValueStr;
			ValueStr += FString::SanitizeFloat(Value.x);
			ValueStr += ",";
			ValueStr += FString::SanitizeFloat(Value.y);
			ValueStr += ",";
			ValueStr += FString::SanitizeFloat(Value.z);
			ValueStr += ",";
			ValueStr += FString::SanitizeFloat(Value.w);
			PresetVal.mValue = TCHAR_TO_ANSI(*ValueStr);
		}
		break;
	case Substance_IOType_Integer:
		{
			int32 Value;
			Ar << Value;

			FString ValueStr = FString::FromInt(Value);
			PresetVal.mValue = TCHAR_TO_ANSI(*ValueStr);
		}
		break;
	case Substance_IOType_Integer2:
		{
			SubstanceAir::Vec2Int Value;
			int32 CurValue;
			Ar << CurValue;
			Value.x = CurValue;
			Ar << CurValue;
			Value.y = CurValue;

			FString ValueStr;
			ValueStr += FString::FromInt(Value.x);
			ValueStr += ",";
			ValueStr += FString::FromInt(Value.y);
			PresetVal.mValue = TCHAR_TO_ANSI(*ValueStr);
		}
		break;
	case Substance_IOType_Integer3:
		{
			SubstanceAir::Vec3Int Value;
			int32 CurValue;
			Ar << CurValue;
			Value.x = CurValue;
			Ar << CurValue;
			Value.y = CurValue;
			Ar << CurValue;
			Value.z = CurValue;

			FString ValueStr;
			ValueStr += FString::FromInt(Value.x);
			ValueStr += ",";
			ValueStr += FString::FromInt(Value.y);
			ValueStr += ",";
			ValueStr += FString::FromInt(Value.z);
			PresetVal.mValue = TCHAR_TO_ANSI(*ValueStr);
		}
		break;
	case Substance_IOType_Integer4:
		{
			SubstanceAir::Vec4Int Value;
			int32 CurValue;
			Ar << CurValue;
			Value.x = CurValue;
			Ar << CurValue;
			Value.y = CurValue;
			Ar << CurValue;
			Value.z = CurValue;
			Ar << CurValue;
			Value.w = CurValue;

			FString ValueStr;
			ValueStr += FString::FromInt(Value.x);
			ValueStr += ",";
			ValueStr += FString::FromInt(Value.y);
			ValueStr += ",";
			ValueStr += FString::FromInt(Value.z);
			ValueStr += ",";
			ValueStr += FString::FromInt(Value.w);
			PresetVal.mValue = TCHAR_TO_ANSI(*ValueStr);
		}
		break;

	case Substance_IOType_Image:
		{
			int32 bHasImage = 0;
			Ar << bHasImage;

			if (bHasImage)
			{
				UObject* ImageSource;
				Ar << ImageSource;

				//Note - Even though this is a UObject, In legacy, the only input Uobjects used are our own (USubstanceImageInput)
				UTexture2D* Image = Cast<UTexture2D>(ImageSource);
				Graph->ImageSources.Add(Input.UID, Image);
			}
		}
		break;
	}
}

FString GuiWidgetStringLabel(int32 Value)
{
	switch (Value)
	{
	case SubstanceAir::InputWidget::Input_Togglebutton:
		return "togglebutton";
	case SubstanceAir::InputWidget::Input_Color:
		return "color";
	case SubstanceAir::InputWidget::Input_Slider:
		return "slider";
	case SubstanceAir::InputWidget::Input_Combobox:
		return "combobox";
	case SubstanceAir::InputWidget::Input_Image:
		return "image";
	default:
		return "";
	};

	return "";
}
