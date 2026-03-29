// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceCoreHelpers.h

#pragma once
#ifdef SUBSTANCE_FRAMEWORK_INCLUDED
#include <substance/pixelformat.h>
#include <substance/framework/graph.h>
#include <substance/framework/preset.h>
#include "substance/framework/typedefs.h"
#endif

#include "Engine/Texture.h"
#include <Materials/Material.h>
#include <Materials/MaterialInstanceConstant.h>
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"

#define MAX_SUBSTANCES_TO_UPDATE 1

class USubstanceInstanceFactory;
class USubstanceGraphInstance;
class USubstanceTexture2D;
class UObject;

//Struct used for capturing material linking data for post reimport
struct MaterialParameterSet
{
	UMaterial* Material;
	TMap<FString, class UMaterialExpressionTextureSample*> ParameterNames;

	//Default constructor and destructor
	MaterialParameterSet() : Material(nullptr) { }
	~MaterialParameterSet() { }

	//Comparison operator for adding unique elements to a TArray
	bool operator==(const MaterialParameterSet& rhs) const
	{
		bool IsEqual = (this->Material == rhs.Material && rhs.ParameterNames.Num() == this->ParameterNames.Num());
		return IsEqual;
	}
};

struct MaterialInstanceParameterSet
{
	UMaterialInstance* MaterialInstance;
	TMap<FString, FName> ParameterNames;

	//Default constructor and destructor
	MaterialInstanceParameterSet() : MaterialInstance(nullptr) { }
	~MaterialInstanceParameterSet() { }

	//Comparison operator for adding unique elements to a TArray
	bool operator==(const MaterialInstanceParameterSet& rhs) const
	{
		bool IsEqual = (this->MaterialInstance == rhs.MaterialInstance && rhs.ParameterNames.Num() == this->ParameterNames.Num());
		return IsEqual;
	}
};

struct OutputTextureSettings
{
	FString PackageName;
	uint8 sRGB_Enabled;

	//Default constructor and destructor
	OutputTextureSettings() : sRGB_Enabled(false) { }
	//OutputTextureSettings(const& FString packageName, uint8 srgbEnabled) : sRGB_Enabled(srgbEnabled), PackageName(packageName) { }
	~OutputTextureSettings() { }
};

//Framework forward declarations
namespace SubstanceAir
{
struct GraphDesc;
class PackageDesc;
class InputImage;
class OutputInstance;
class InputInstanceBase;
class InputInstanceImage;
class InputInstanceNumericalBase;
class ImageInput;
class GraphInstance;
}

//A collection of helper functions used for both editor and core
namespace Substance
{
struct RenderCallbacks;

/** Used for rebuilding instances within substance factory */
struct InstanceBackup
{
	SubstanceAir::Preset preset;
	USubstanceGraphInstance* InstanceParent;
	TMap<uint32, FString> UidToName;
};

/** Contains helper functions to call from the main thread. */
namespace Helpers
{
SUBSTANCECORE_API RenderCallbacks* GetRenderCallbacks();

/** Perform an asynchronous (non-blocking) rendering of those instances */
SUBSTANCECORE_API void RenderAsync(TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>>&);
SUBSTANCECORE_API void RenderAsync(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>);

/** Perform a blocking rendering of those instances */
SUBSTANCECORE_API void RenderSync(TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>>&, bool ForceCache = false);
SUBSTANCECORE_API void RenderSync(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>, bool ForceCache = false);

/** UE4 Play in editor callback */
SUBSTANCECORE_API void StartPIE();
SUBSTANCECORE_API void EndPIE();

/** Queue the graph instance for later render -- Used during seek free loading */
SUBSTANCECORE_API void PushDelayedRender(USubstanceGraphInstance* Instance);
SUBSTANCECORE_API void PushDelayedRender(SubstanceAir::InputInstanceImage* ImgInput, USubstanceGraphInstance* Instance);

/** Queue an image input until its source has been rendered */
SUBSTANCECORE_API void PushDelayedImageInput(SubstanceAir::InputInstanceImage* ImgInput, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance);
SUBSTANCECORE_API void RemoveFromDelayedImageInputs(SubstanceAir::InputInstanceImage* ImgInput);

/** UE4 level callbacks */
void OnWorldInitialized();
void OnLevelAdded();

//! @brief Acquire the most recent render result from an output
//! @param output Substance framework output object
//! @return Pointer to the render result if available, nullptr if empty
//! @note If there are older results, they will be cleared and freed
SubstanceAir::OutputInstance::Result getNewestOutputResult(SubstanceAir::OutputInstance* output);

/** Render queued graph instances */
void PerformDelayedRender();

/** Update Texture Output */
void UpdateTexture(const SubstanceTexture& result, SubstanceAir::OutputInstance* Output, bool bCacheResults = true);

/** Perform per frame Substance management */
SUBSTANCECORE_API void Tick();

/** Create a texture 2D object using an output instance */
SUBSTANCECORE_API void CreateSubstanceTexture2D(SubstanceAir::OutputInstance* output, bool bTransient = false,
        FString Name = FString(), UObject* InOuter = nullptr, USubstanceGraphInstance* ParentInst = nullptr);

/** Initializes 4 levels of mip maps */
SUBSTANCECORE_API void CreatePlaceHolderMips(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance);

/** Convert from a Substance Format to UE4 equivalent */
EPixelFormat SubstanceToUe4Format(const SubstancePixelFormat Format, const SubstanceChannelsOrder ChanOrder);

void OverwriteSubstancePixelFormatForSourceImage(SubstanceAir::OutputInstance* Output);
void OverwriteSubstancePixelFormatForRuntimeCompression(SubstanceAir::OutputInstance* Output);
ETextureSourceFormat SubstanceToUE4SourceFormat(const SubstancePixelFormat Format);

/** Convert from a UE4 pixel format to a Substance Format */
SubstancePixelFormat UE4FormatToSubstance(EPixelFormat Fmt);

/** Creates and enables a texture object*/
SUBSTANCECORE_API void EnableTexture(SubstanceAir::OutputInstance* Output, USubstanceGraphInstance* Graph, const OutputTextureSettings* TextureSettings = nullptr, bool IsTransient = false);

/** Create textures of empty output instances of the given graph instance */
SUBSTANCECORE_API void CreateTextures(USubstanceGraphInstance* Graph, bool ForceCreate = false, bool IsTransient = false);

/** Update the outputs of each instance */
SUBSTANCECORE_API bool UpdateTextures(TArray<SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>>& Instances, bool InEditor = false);
SUBSTANCECORE_API bool UpdateTextures(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, bool InEditor = false);

/** Helper to retrieve the outputinstance from the framework */
SUBSTANCECORE_API SubstanceAir::OutputInstance* GetSubstanceOutputByID(USubstanceGraphInstance* graph, uint32 UID);

/** Compare the values of two input instances. */
bool AreInputValuesEqual(SubstanceAir::InputInstanceBase& a, SubstanceAir::InputInstanceBase& b);

/** Updates Input values */
SUBSTANCECORE_API int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, class UObject* InValue);
SUBSTANCECORE_API int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, const FString& InValue);
SUBSTANCECORE_API int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, const TArray<int32>& InValue);
SUBSTANCECORE_API int32 UpdateInput(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const SubstanceAir::InputInstanceBase* InputInstance, const TArray<float>& InValue);
int32 UpdateInputHelper(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Instance, const uint32& Uid);

template <typename T> void SetNumericalInputValueHelper(const SubstanceAir::InputInstanceNumericalBase* BaseInput, const TArray<T>& InValue);
SUBSTANCECORE_API void SetNumericalInputValue(const SubstanceAir::InputInstanceNumericalBase* BaseInput, const TArray<int>& InValue);
SUBSTANCECORE_API void SetNumericalInputValue(const SubstanceAir::InputInstanceNumericalBase* BaseInput, const TArray<float>& InValue);
SUBSTANCECORE_API void SetStringInputValue(const SubstanceAir::InputInstanceBase* BaseInput, const FString& InValue);

/** Return the string representation of the Input's value */
SUBSTANCECORE_API FString GetValueAsString(const SubstanceAir::InputInstanceBase& Input);

/** Return the value of a float input */
TArray<float> GetValueFloat(const SubstanceAir::InputInstanceBase& Input);

/** Return the value of a int input */
TArray<int32> GetValueInt(const SubstanceAir::InputInstanceBase& Input);

/** Returns a string value of a string input*/
FString GetValueString(const SubstanceAir::InputInstanceBase& Input);

/** Setup the rendering system for commandlet or things like that */
void SetupSubstance(void* library);

/** Cleans up the rendering system */
void TearDownSubstance();

/** Disable an output -- This will delete the outputs texture */
SUBSTANCECORE_API void Disable(SubstanceAir::OutputInstance* output, bool FlagToDelete = true);

/** Find which desc an instance belongs to */
const SubstanceAir::GraphDesc* FindParentGraph(const std::vector<SubstanceAir::GraphDesc>& Graphs, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>);
const SubstanceAir::GraphDesc* FindParentGraph(const std::vector<SubstanceAir::GraphDesc>& Graphs, const FString& URL);

/** Sets up an InputImage and returns the shared pointer */
SubstanceAir::shared_ptr<SubstanceAir::InputImage> PrepareImageInput(class UObject* InValue,
        SubstanceAir::InputInstanceImage* Input, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Parent);

/** Calculates image size per mip level - UE4's implementation doesn't support low mips leves! */
SIZE_T CalculateSubstanceImageBytes(uint32 SizeX, uint32 SizeY, uint32 SizeZ, uint8 Format);

/** Checks to see if a format type is currently supported */
bool IsSupportedImageInput(UObject*);

/** Sets the properties of an Image Input Instance */
void SetImageInput(SubstanceAir::InputInstanceImage* Input, UObject* InValue, SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Parent,
                   bool unregisterOutput = true, bool isTransacting = false);

/** Clears a graph instance from the Renderer */
void ClearFromRender(USubstanceGraphInstance* GraphInstance);

/** Resets the graph instance image inputs */
void ResetGraphInstanceImageInputs(USubstanceGraphInstance* GraphInstance);

/** brief Create an Unreal Material for the given graph-instance */
SUBSTANCECORE_API TWeakObjectPtr<UMaterialInstance> CreateMaterial(USubstanceGraphInstance* Instance, const FString& MatName, UObject* Outer = nullptr);

/** Generate a list of material expressions and map them to existing graph instance outputs */
SUBSTANCECORE_API void GenerateMaterialExpressions(SubstanceAir::GraphInstance* Instance, UMaterialInstance* MaterialInstance, USubstanceGraphInstance* Graph, bool IsOnImport = false);

/** Generate a new simple parent material to use as a template for instancing 
	Creates parameter nodes based on substance outputs and connects them to best matching output nodes*/
SUBSTANCECORE_API UMaterial* GenerateTemplateMaterial(USubstanceGraphInstance* Instance, const FString& MaterialName, UObject* Outer);

/** Create the sampler nodes in UMaterial graph for legacy material generation*/
SUBSTANCECORE_API UMaterialExpressionTextureSampleParameter2D* CreateSampler(UMaterial* UnrealMaterial, UTexture* UnrealTexture, const SubstanceAir::OutputDesc* OutputDesc);

SUBSTANCECORE_API void CreateMaterialExpression(SubstanceAir::OutputInstance* OutputInst, UMaterial* UnrealMaterial);

/** Set material instance expression values
	Texture Parameters - Called once to set initial texture pointer on material creation
	Value Parameters - Called whenever a value output is updated */
SUBSTANCECORE_API void SetMaterialExpression(SubstanceAir::OutputInstance* OutputInst, UMaterialInstance* UnrealMaterial, FMaterialParameterInfo &Info);

/** Creates and fills out a FLinearColor(float4) with data from output result to use in a material parameter node
	for use with substance vec2, vec3, and vec4 of int or float*/
SUBSTANCECORE_API FLinearColor SubstanceIOVectorToParameter(SubstanceAir::RenderResultNumericalBase* NumericOutput);

/** Sets all of the material inputs to the proper outputs from a graph instance */
SUBSTANCECORE_API void ResetMaterialTexturesFromGraph(USubstanceGraphInstance* Graph, UMaterialInstanceConstant* OwnedMaterial, const TArray<MaterialParameterSet>& Materials);

/** Sets all of the material instance parameters to the proper outputs from a graph instance */
SUBSTANCECORE_API void ResetMaterialInstanceTexturesFromGraph(USubstanceGraphInstance* Graph, const TArray<MaterialInstanceParameterSet>& Materials);

/** Reset an instance's inputs to their default values -- Does not trigger rendering of the instance */
SUBSTANCECORE_API void ResetToDefault(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance>);

/** Reset an instance's input to its default value -- Does no trigger rendering of the instance */
SUBSTANCECORE_API void ResetToDefault(SubstanceAir::InputInstanceBase* Input);

/** Fills out string values with a unique name */
SUBSTANCECORE_API void GetSuitableName(SubstanceAir::OutputInstance* Instance,
                                       FString& OutTextureName, FString& OutPackageName, USubstanceGraphInstance* OptionalParentGraph = nullptr);

/** Create instance of a graph desc */
SUBSTANCECORE_API USubstanceGraphInstance* InstantiateGraph(USubstanceInstanceFactory* ParentFactory, const SubstanceAir::GraphDesc& Graph, UObject* Outer,
        FString InstanceName = FString(), bool bCreateOutputs = false, EObjectFlags Flags = RF_Standalone | RF_Public, UMaterial* InParentMaterial = nullptr);

/** Copy constructor for USubstanceGraphInstance */
USubstanceGraphInstance* DuplicateGraphInstance(USubstanceGraphInstance* Instance);

/** Copy constructor for USubstanceGraphInstance */
void CopyInstance(USubstanceGraphInstance* RefInstance, USubstanceGraphInstance* NewInstance, bool bCopyOutputs = true);

/** Queues different wrapper types for cleanup */
SUBSTANCECORE_API void RegisterForDeletion(class USubstanceGraphInstance* InstanceContainer);
SUBSTANCECORE_API void RegisterForDeletion(class UTexture2D* InstanceContainer);
SUBSTANCECORE_API void RegisterForDeletion(class USubstanceInstanceFactory* InstanceContainer);
SUBSTANCECORE_API void PerformDelayedDeletion();

/** Creates a package for an object */
SUBSTANCECORE_API UPackage* CreateObjectPackage(UObject* Outer, FString ObjectName);

/** Returns the loading progress percentage */
float GetSubstanceLoadingProgress();

/** Creates a substance air package desc and return it - This is here for the editor module to use*/
SUBSTANCECORE_API SubstanceAir::shared_ptr<SubstanceAir::PackageDesc> InstantiatePackage(void* assemData, uint32 size);

/** Assigns a graph instance input deltas to the preset passed in */
SUBSTANCECORE_API void CopyPresetData(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Source, SubstanceAir::Preset& Dest);

/** Get empty preset as shared */
SUBSTANCECORE_API SubstanceAir::shared_ptr<SubstanceAir::Preset> EmptyPresetAsShared();

/** Assigns a graph instance input deltas to the preset passed in */
SUBSTANCECORE_API void CopyPresetData(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Source, SubstanceAir::Preset& Dest);

/** Apply Preset data to a graph instance*/
SUBSTANCECORE_API void ApplyPresetData(SubstanceAir::shared_ptr<SubstanceAir::GraphInstance> Destination, SubstanceAir::Preset& Source);

/** Exposes the frameworks instantiate graphs for the editor*/
SUBSTANCECORE_API void InstatiateGraphsFromPackage(SubstanceAir::GraphInstances& Instances, SubstanceAir::PackageDesc* Package);

/** Returns an XML formatted string with the current preset values*/
SUBSTANCECORE_API FString GetPresetAsXMLString(USubstanceGraphInstance*);

/** Exports input deltas from graph instance */
SUBSTANCECORE_API bool ExportPresetFromGraph(USubstanceGraphInstance*);

/** Loads and sets input deltas for graph instance */
SUBSTANCECORE_API bool ImportAndApplyPresetForGraph(USubstanceGraphInstance*);

/** Imports a preset file */
SUBSTANCECORE_API FString ImportPresetFile();

/** Exports a preset file */
SUBSTANCECORE_API FString ExportPresetFile(FString SuggestedFilename);

/** Load and apply presets to a given graph instance*/
SUBSTANCECORE_API void ParsePresetAndApply(SubstanceAir::Presets& CurrentPresets, FString& xmlString, USubstanceGraphInstance* Graph);

/** Saves the current input deltas to a file */
SUBSTANCECORE_API void WriteSubstanceXmlPreset(SubstanceAir::Preset& preset, FString& XmlContent);

/** Clear current cache to free up memory */
SUBSTANCECORE_API void ClearCache();

/** Returns an array of the material templates included with the substance plugin (may return none if the editor has not fully loaded yet) */
SUBSTANCECORE_API TArray<UMaterial*> GetSubstanceIncludedMaterials();

#if WITH_EDITOR
/** Recomputes all substance graph instances! - Only available from editor */
SUBSTANCECORE_API uint32 RebuildAllSubstanceGraphInstances();

/** Creates a material with a default formatted unique material name */
SUBSTANCECORE_API void CreateDefaultNamedMaterial(USubstanceGraphInstance* Graph, FString Extention = FString());

/** Checks if any data is missing from the substances loaded in the project, used to notify editormodule to rebuild all substances */
SUBSTANCECORE_API bool SubstancesRequireUpdate();
#endif // WITH_EDITOR

} // namespace Helpers
} // namespace Substance
