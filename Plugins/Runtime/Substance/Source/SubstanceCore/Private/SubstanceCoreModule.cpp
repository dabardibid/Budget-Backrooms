// Copyright 2018 Allegorithmic Inc. All rights reserved.
// File: SubstanceCoreModule.cpp

#include "SubstanceCoreModule.h"
#include "SubstanceCorePrivatePCH.h"
#include "SubstanceCoreHelpers.h"

#include "SubstanceCoreClasses.h"
#include "SubstanceStyle.h"

#include "AssetRegistryModule.h"
#include "Modules/ModuleManager.h"

#include "Misc/MessageDialog.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#define LOCTEXT_NAMESPACE "SubstanceCoreModule"

#if SUBSTANCE_MEMORY_STAT
DEFINE_STAT(STAT_SubstanceCacheMemory);
DEFINE_STAT(STAT_SubstanceEngineMemory);
DEFINE_STAT(STAT_SubstanceImageMemory);
#endif

DEFINE_LOG_CATEGORY_STATIC(LogSubstanceCoreModule, Log, All);

//Resolve library file name depending on build platform
#if defined (SUBSTANCE_ENGINE_DYNAMIC)
#	if defined(SUBSTANCE_ENGINE_DEBUG)
#		define SUBSTANCE_LIB_CONFIG "Debug"
#	else
#		define SUBSTANCE_LIB_CONFIG "Release"
#	endif
#	if PLATFORM_WINDOWS
#		if PLATFORM_64BITS
#			if _MSC_VER < 1900
#				define SUBSTANCE_LIB_CPU_DYNAMIC_PATH	TEXT("DLLs/" SUBSTANCE_LIB_CONFIG "/Win64_2013/substance_sse2_blend.dll")
#				define SUBSTANCE_LIB_GPU_DYNAMIC_PATH	TEXT("DLLs/" SUBSTANCE_LIB_CONFIG "/Win64_2013/substance_d3d11pc_blend.dll")
#			else
#				define SUBSTANCE_LIB_CPU_DYNAMIC_PATH	TEXT("DLLs/" SUBSTANCE_LIB_CONFIG "/Win64/substance_sse2_blend.dll")
#				define SUBSTANCE_LIB_GPU_DYNAMIC_PATH	TEXT("DLLs/" SUBSTANCE_LIB_CONFIG "/Win64/substance_d3d11pc_blend.dll")
#			endif
#		else
#			error Unsupported platform for dynamic substance loading
#		endif
#   elif PLATFORM_MAC
#       define SUBSTANCE_LIB_CPU_DYNAMIC_PATH TEXT("DLLs/" SUBSTANCE_LIB_CONFIG "/Mac/libsubstance_sse2_blend.dylib")
#       define SUBSTANCE_LIB_GPU_DYNAMIC_PATH TEXT("DLLs/" SUBSTANCE_LIB_CONFIG "/Mac/libsubstance_ogl3_blend.dylib")
#   elif PLATFORM_LINUX
#		define SUBSTANCE_LIB_CPU_DYNAMIC_PATH TEXT("DLLs/" SUBSTANCE_LIB_CONFIG "/Linux/libsubstance_sse2_blend.so")
#		define SUBSTANCE_LIB_GPU_DYNAMIC_PATH TEXT("DLLs/" SUBSTANCE_LIB_CONFIG "/Linux/libsubstance_ogl3_blend.so")
#   else
#       error Unsupported platform for dynamic substance loading
#	endif
#endif

namespace
{
static FWorldDelegates::FWorldInitializationEvent::FDelegate OnWorldInitDelegate;
static FDelegateHandle OnWorldInitDelegateHandle;

static FWorldDelegates::FOnLevelChanged::FDelegate OnLevelAddedDelegate;
static FDelegateHandle OnLevelAddedDelegateHandle;
}

void FSubstanceCoreModule::RegisterSettings()
{
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "Substance",
		                                 LOCTEXT("SubstanceSettingsName", "Substance"),
		                                 LOCTEXT("SubstanceSettingsDescription", "Configure the Substance plugin"),
		                                 GetMutableDefault<USubstanceSettings>()
		                                );
	}
#endif
}

void FSubstanceCoreModule::UnregisterSettings()
{
#if WITH_EDITOR
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "Substance");
	}
#endif
}

void FSubstanceCoreModule::StartupModule()
{
	void* libraryPtr = nullptr;

#if defined (SUBSTANCE_ENGINE_DYNAMIC)
	const TCHAR* libraryFileName =
	    (GetDefault<USubstanceSettings>()->SubstanceEngine == ESubstanceEngineType::SET_CPU)
	    ? SUBSTANCE_LIB_CPU_DYNAMIC_PATH
	    : SUBSTANCE_LIB_GPU_DYNAMIC_PATH;

	//Previous plugin paths used to test if the user has manually installed the plugin in the wrong path
	FString previousPrefixPaths[] =
	{
		FPaths::EnginePluginsDir(),
		FPaths::ProjectPluginsDir(),
	};

	//The plugin can exist in a few different places, so lets try each one in turn
	FString prefixPaths[] =
	{
		FPaths::Combine(*FPaths::ProjectPluginsDir(), TEXT("Runtime/")),
		FPaths::Combine(*FPaths::EnginePluginsDir(), TEXT("Runtime/")),
		FPaths::Combine(*FPaths::EnginePluginsDir(), TEXT("Marketplace/")),
	};

	//Check old plugin path locations to see if the installation is correct
	size_t numPreviousPaths = sizeof(previousPrefixPaths) / sizeof(FString);
	for (size_t i = 0; i < numPreviousPaths; i++)
	{
		FString libraryPath = FPaths::Combine(*previousPrefixPaths[i], TEXT("Substance/"), libraryFileName);
		if (FPaths::FileExists(libraryPath))
		{
			FText MessageBoxTitle = FText::FromString("Substance Installation Error");
			FText ErrorMessage;

			if (previousPrefixPaths[i] == FPaths::EnginePluginsDir())
			{
				ErrorMessage = LOCTEXT("Substance Install Error in Engine", "The Substance Plugin was not installed to the expected folder. \n Current Path: <Engine-Directory>/Plugins/Substance \n Valid path:   <Engine-Directory>/Plugins/Runtime/Substance");
			}
			else if (previousPrefixPaths[i] == FPaths::ProjectPluginsDir())
			{
				ErrorMessage = LOCTEXT("Substance Install Error in Project", "The Substance Plugin was not installed to the expected folder. \n Current Path: <Project-Directory>/Plugins/Substance \n Valid path:   <Project-Directory>/Plugins/Runtime/Substance");
			}

			//Alert user that the path is wrong
			FMessageDialog::Open(EAppMsgType::Ok, ErrorMessage, &MessageBoxTitle);

			//End the for loop as we don't want to display this message multiple times
			break;
		}
	}

	//Search current locations of the plugin to load the correct lib
	size_t numPaths = sizeof(prefixPaths) / sizeof(FString);
	for (size_t i = 0; i < numPaths; i++)
	{
		FString libraryPath = FPaths::Combine(*prefixPaths[i], TEXT("Substance/"), libraryFileName);
		if (FPaths::FileExists(libraryPath))
		{
			if (void* pLibraryHandle = FPlatformProcess::GetDllHandle(*libraryPath))
			{
				libraryPtr = pLibraryHandle;
				break;
			}
		}
	}

	if (libraryPtr == nullptr)
	{
		UE_LOG(LogSubstanceCoreModule, Fatal, TEXT("Unable to load Substance Library."));
	}
#endif

	if (GetDefault<USubstanceSettings>()->SubstanceEngine == ESubstanceEngineType::SET_CPU)
	{
		UE_LOG(LogSubstanceCoreModule, Log, TEXT("Substance [CPU] Engine Loaded, Max Texture Size = 2048"));
	}
	else
	{
		UE_LOG(LogSubstanceCoreModule, Log, TEXT("Substance [GPU] Engine Loaded, Max Texture Size = 8192"));
	}

	//Register tick function
	if (!IsRunningDedicatedServer())
	{
		//Init substance core objects
		Substance::Helpers::SetupSubstance(libraryPtr);

		TickDelegate = FTickerDelegate::CreateRaw(this, &FSubstanceCoreModule::Tick);
		FTicker::GetCoreTicker().AddTicker(TickDelegate);

		RegisterSettings();

		::OnWorldInitDelegate = FWorldDelegates::FWorldInitializationEvent::FDelegate::CreateStatic(&FSubstanceCoreModule::OnWorldInitialized);
		::OnWorldInitDelegateHandle = FWorldDelegates::OnPostWorldInitialization.Add(::OnWorldInitDelegate);

		::OnLevelAddedDelegate = FWorldDelegates::FOnLevelChanged::FDelegate::CreateStatic(&FSubstanceCoreModule::OnLevelAdded);
		::OnLevelAddedDelegateHandle = FWorldDelegates::LevelAddedToWorld.Add(::OnLevelAddedDelegate);

#if WITH_EDITOR
		FEditorDelegates::BeginPIE.AddRaw(this, &FSubstanceCoreModule::OnBeginPIE);
		FEditorDelegates::EndPIE.AddRaw(this, &FSubstanceCoreModule::OnEndPIE);
		FSubstanceStyle::Initialize();
#endif //WITH_EDITOR

	}

	PIE = false;

//Part of the debug suite - Fully loads all substance graph instances and substance textures on launch
#if WITH_EDITOR && SUBSTANCE_CORE_DEBUG_TOOLS
	//NOTE:: This grabs a library of all of the substance graph instances and "checks them out" all at load time
	//similar to how it will check out each one when moused over in the content browser. This allows us to fully load
	//all graph instances at load time preventing issues with legacy serialization -> Refactor serialization.

	TWeakObjectPtr<UObjectLibrary> ObjectGraphLibrary = UObjectLibrary::CreateLibrary(USubstanceGraphInstance::StaticClass(), false, GIsEditor);
	if (ObjectGraphLibrary.IsValid())
	{
		ObjectGraphLibrary->AddToRoot();
		FString NewPath = TEXT("/Game");
		int32 NumOfAssetDatas = ObjectGraphLibrary->LoadAssetDataFromPath(NewPath);
		TArray<FAssetData> AssetData;
		ObjectGraphLibrary->GetAssetDataList(AssetData);

		TWeakObjectPtr<UObject> Asset;
		for (int32 i = 0; i < AssetData.Num(); ++i)
		{
			AssetData[i].GetAsset();
		}
	}

	//Load all of the texture2D assets from the library (Forces thumbnail update and has the same functionality as mouse over asset thumbnail in editor)
	TWeakObjectPtr<UObjectLibrary> ObjectTextureLibrary = UObjectLibrary::CreateLibrary(USubstanceTexture2D::StaticClass(), false, GIsEditor);
	if (ObjectTextureLibrary.IsValid())
	{
		ObjectTextureLibrary->AddToRoot();
		FString NewPath = TEXT("/Game");
		int32 NumOfAssetDatas = ObjectTextureLibrary->LoadAssetDataFromPath(NewPath);
		TArray<FAssetData> AssetData;
		ObjectTextureLibrary->GetAssetDataList(AssetData);

		for (int32 i = 0; i < AssetData.Num(); ++i)
		{
			AssetData[i].GetAsset();
		}
	}

#endif //WITH_EDITOR
}

void FSubstanceCoreModule::ShutdownModule()
{
	FSubstanceStyle::Shutdown();
	FWorldDelegates::OnPostWorldInitialization.Remove(::OnWorldInitDelegateHandle);
	FWorldDelegates::LevelAddedToWorld.Remove(::OnLevelAddedDelegateHandle);
	FTicker::GetCoreTicker().RemoveTicker(TickDelegate.GetHandle());
	UnregisterSettings();

	Substance::Helpers::TearDownSubstance();
}

bool FSubstanceCoreModule::Tick(float DeltaTime)
{
	Substance::Helpers::Tick();

	return true;
}

uint32 FSubstanceCoreModule::GetMaxOutputTextureSize() const
{
	if (GetDefault<USubstanceSettings>()->SubstanceEngine == ESubstanceEngineType::SET_GPU)
		return 8192;
	else
		return 4096;
}

void FSubstanceCoreModule::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS)
{
	Substance::Helpers::OnWorldInitialized();
}

void FSubstanceCoreModule::OnLevelAdded(ULevel* Level, UWorld* World)
{
	Substance::Helpers::OnLevelAdded();
}

void FSubstanceCoreModule::LogSubstanceSettings()
{
	//Logs CPU Core count
	UE_LOG(LogSubstanceCore, Warning, TEXT("Core Count: %d"), GetDefault<USubstanceSettings>()->CPUCores);

	//Log Memory Budget
	UE_LOG(LogSubstanceCore, Warning, TEXT("Memory Budget: %d"), GetDefault<USubstanceSettings>()->MemoryBudgetMb);

	//Log Async Mip Limit
	UE_LOG(LogSubstanceCore, Warning, TEXT("Async Mip Clip: %d"), GetDefault<USubstanceSettings>()->AsyncLoadMipClip);

	//Log Max Async Substance Rendered Per Frame
	UE_LOG(LogSubstanceCore, Warning, TEXT("Max Async Render Per Frame: %d"), GetDefault<USubstanceSettings>()->MaxAsyncSubstancesRenderedPerFrame);

	//Log which Substance Engine is in use
	if (GetDefault<USubstanceSettings>()->SubstanceEngine == ESubstanceEngineType::SET_CPU)
	{
		UE_LOG(LogSubstanceCore, Warning, TEXT("Substance CPU Engine Set"));
	}
	else
	{
		UE_LOG(LogSubstanceCore, Warning, TEXT("Substance GPU Engine Set"));
	}
}

#if WITH_EDITOR
void FSubstanceCoreModule::OnBeginPIE(const bool bIsSimulating)
{
	PIE = true;
}

void FSubstanceCoreModule::OnEndPIE(const bool bIsSimulating)
{
	PIE = false;
}

bool FSubstanceCoreModule::isPie() const
{
	{
		return PIE;
	}
}

#endif

IMPLEMENT_MODULE(FSubstanceCoreModule, SubstanceCore);

#undef LOCTEXT_NAMESPACE
