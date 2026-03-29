// Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;
using System.Diagnostics;
#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

namespace UnrealBuildTool.Rules
{
public class SubstanceCore : ModuleRules
{
	private string ModulePath
	{
		get { return ModuleDirectory; }
	}

	private string PluginRootPath
	{
		get { return Path.GetFullPath(Path.Combine(ModulePath, "../../")); }
	}

	private string SubstanceIncludePath
	{
		get { return Path.GetFullPath(Path.Combine(PluginRootPath, "Include")); }
	}

	private string ModuleFullPath
	{
		get { return Path.GetFullPath(ModulePath); }
	}

	public SubstanceCore(ReadOnlyTargetRules Target) : base(Target)
	{
		//Internal defines
		PublicDefinitions.Add("WITH_SUBSTANCE=1");
		PublicDefinitions.Add("SUBSTANCE_PLATFORM_BLEND=1");
		PublicDefinitions.Add("SUBSTANCE_CORE_DEBUG_TOOLS=0");

		//PCH file
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;
		PrivatePCHHeaderFile = "Private/SubstanceCorePrivatePCH.h";

		//Thread usage
		if (Target.IsInPlatformGroup(UnrealPlatformGroup.Microsoft)) // Windows, Xbox
		{
			PublicDefinitions.Add("AIR_USE_WIN32_SYNC=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac ||
		         Target.Platform == UnrealTargetPlatform.PS4 ||
		         Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicDefinitions.Add("AIR_USE_PTHREAD_SYNC=1");
		}

		//Exposing the include path to substance core publicly. Even though the include paths are public, the libs should no longer be exposed.
		PrivateIncludePaths.Add("SubstanceCore/Private");

		//Exposing public include paths
		PublicIncludePaths.Add(Path.Combine(ModuleFullPath, "Public"));
		PublicIncludePaths.Add(Path.Combine(ModuleFullPath, "Classes"));
		PublicIncludePaths.Add(SubstanceIncludePath);
		PrecompileForTargets = PrecompileTargetsType.Any;

		//Module dependencies
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Projects",
			"Slate",
			"SlateCore",
			"SubstanceEngine"
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"AssetRegistry",
			"Core",
			"CoreUObject",
			"Engine",
			"RenderCore",
			"RHI",
			"ImageWrapper",
			"SessionServices",
		});

		//Editor specific configuration
		bool IncludePS4Files = false;

		if (Target.bBuildEditor == true)
		{
			PublicDependencyModuleNames.AddRange(new string[]
			{
				"UnrealEd",
				"AssetTools",
				"ContentBrowser",
				"Settings",
				"TargetPlatform",
				"MainFrame"
			});

			//Used for ps4 cooking
			string SDKDir = System.Environment.GetEnvironmentVariable("SCE_ORBIS_SDK_DIR");
			if ((SDKDir != null) && (SDKDir.Length > 0))
			{
				PublicIncludePaths.Add(SDKDir + "/target/include_common");

				PublicAdditionalLibraries.Add(Path.Combine(SDKDir, "host_tools", "lib", "libSceGpuAddress.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(SDKDir, "host_tools", "lib", "libSceGnm.lib"));

				PublicDelayLoadDLLs.Add("libSceGpuAddress.dll");
				PublicDelayLoadDLLs.Add("libSceGnm.dll");

				//Toggle on our flag if we are building for PS4
				IncludePS4Files = true;
			}
		}

		//Overwrite PS4 SDK if the files don't exist - Check both Engine and Project directories
		string ConsoleFilePath = "SubstanceCore\\Private\\SubstanceCorePS4Utils.h";
		string BaseEnginePath = Path.Combine("..", "Plugins", "Runtime", "Substance", "Source");
		string BaseProjectPath = Path.Combine("Plugins", "Runtime", "Substance", "Source");
		string ConsoleEngineFilePath = Path.Combine(BaseEnginePath, ConsoleFilePath);

		bool ConsoleFilesFoundInEnginePluginDir = false;
		bool ConsoleFilesFoundInProjectPluginDir = false;

		//If building plugin from engine dir, project file will not exist
		if (Target.ProjectFile != null)
		{
			string ProjectParentDir = System.IO.Directory.GetParent(Target.ProjectFile.ToString()).ToString();
			string ConsoleProjectFilePath = Path.Combine(ProjectParentDir, BaseProjectPath, ConsoleFilePath);
			ConsoleFilesFoundInProjectPluginDir = File.Exists(ConsoleProjectFilePath) && IncludePS4Files;
		}

		//Check both possible locations of where the console files could be
		ConsoleFilesFoundInEnginePluginDir = File.Exists(ConsoleEngineFilePath) && IncludePS4Files;

		//Craft the project file location to test as well
		IncludePS4Files = (ConsoleFilesFoundInEnginePluginDir || ConsoleFilesFoundInProjectPluginDir) && IncludePS4Files;

		PublicDefinitions.Add("SUBSTANCE_HAS_PS4_SDK=" + (IncludePS4Files ? "1" : "0"));

		if (IncludePS4Files)
			Log.WriteLine(LogEventType.Log, "Substance Editor Plugin: PS4 Cooking Enabled");
	}
}
}
