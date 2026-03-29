// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;

public class SubstanceEngine : ModuleRules
{
	/**
	* This modules purpose is to link the Allegorithmic third party libraries for the substance engine
	* The reason this is not handled within substance core is because libraries can only be added publicly and we do not
	* want other modules that will depend on substance core to also add the libraries. This module should only be added as a
	* Private Dependency as to not export the libs to other modules.
	**/

	private enum SubstanceEngineType
	{
		CPU,
		GPU,
	};

	public SubstanceEngine(ReadOnlyTargetRules Target) : base(Target)
	{
		//Marked as an external module as we are not building any source code with this module
		Type = ModuleType.External;

		string BuildConfig = GetBuildConfig(Target);
		string PlatformConfig = GetPlatformConfig(Target);
		string SubstanceLibPath = ModuleDirectory + "/../../Libs/" + BuildConfig + "/" + PlatformConfig + "/";

		Dictionary<SubstanceEngineType, string> EngineLibs = new Dictionary<SubstanceEngineType, string>();
		List<string> StaticLibs = new List<string>();

		//add linker libraries
		StaticLibs.Add("pfxlinkercommon");
		StaticLibs.Add("algcompression");
		StaticLibs.Add("tinyxml");

		string LibExtension = "";
		string LibPrefix = "";
		if (Target.IsInPlatformGroup(UnrealPlatformGroup.Windows))
		{
			LibExtension = ".lib";

			StaticLibs.Add("substance_linker_static");

			EngineLibs.Add(SubstanceEngineType.CPU, "substance_sse2_blend_static");
			EngineLibs.Add(SubstanceEngineType.GPU, "substance_d3d11pc_blend_static");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac ||
		         Target.Platform == UnrealTargetPlatform.Linux)
		{
			LibExtension = ".a";
			LibPrefix = "lib";

			StaticLibs.Add("substance_linker");

			EngineLibs.Add(SubstanceEngineType.CPU, "substance_sse2_blend");
			EngineLibs.Add(SubstanceEngineType.GPU, "substance_ogl3_blend");
		}
		else if (Target.Platform == UnrealTargetPlatform.PS4)
		{
			LibExtension = ".a";
			LibPrefix = "lib";

			StaticLibs.Add("substance_linker");

			EngineLibs.Add(SubstanceEngineType.CPU, "substance_avx_blend");
		}
		else if (Target.Platform.IsInGroup(UnrealPlatformGroup.XboxCommon))
		{
			LibExtension = ".lib";

			StaticLibs.Add("substance_linker");

			EngineLibs.Add(SubstanceEngineType.CPU, "substance_avx_blend");
		}
		else
		{
			throw new BuildException("Substance Plugin does not support platform " + Target.Platform.ToString());
		}

		if (Target.Type == TargetRules.TargetType.Editor)
		{
			PublicDefinitions.Add("AIR_NO_DEFAULT_ENGINE");
			PublicDefinitions.Add("SUBSTANCE_ENGINE_DYNAMIC");

			StaticLibs.Add("substance_framework_editor");

			if (BuildConfig == "Debug")
				PublicDefinitions.Add("SUBSTANCE_ENGINE_DEBUG");
		}
		else
		{
			PublicDefinitions.Add("AIR_NO_DYNLOAD");

			SubstanceEngineType type = GetEngineSuffix(Target);

			if (EngineLibs.ContainsKey(type))
			{
				StaticLibs.Add(EngineLibs[type]);
			}
			else
			{
				StaticLibs.Add(EngineLibs[SubstanceEngineType.CPU]);
			}

			StaticLibs.Add("substance_framework");

			//Linux GPU requires certain libraries added
			if (Target.Platform == UnrealTargetPlatform.Linux && type == SubstanceEngineType.GPU)
			{
				PublicAdditionalLibraries.AddRange(new string[] { "X11", "glut", "GLU", "GL" });
			}
		}

		//add our static libs
		foreach (string staticlib in StaticLibs)
		{
			string libname = LibPrefix + staticlib + LibExtension;
			PublicAdditionalLibraries.Add(SubstanceLibPath + libname);
		}
	}

	private SubstanceEngineType GetEngineSuffix(ReadOnlyTargetRules Target)
	{
		//initialize inipath to main project directory
		string iniPath = "";

		if (Target.ProjectFile != null)
			iniPath = Target.ProjectFile.Directory.FullName;

		//no INI, default to CPU
		if (iniPath.Length == 0)
			return SubstanceEngineType.CPU;

		//We can't use the ConfigCacheIni file here because it bypasses the Substance section.
		try
		{
			iniPath = Path.Combine(iniPath, Path.Combine("Config", "DefaultEngine.ini"));

			//parse INI file line by line until we find our result
			StreamReader file = new StreamReader(iniPath);
			string line;
			string startsWith = "SubstanceEngine=";
			while ((line = file.ReadLine()) != null)
			{
				if (line.StartsWith(startsWith))
				{
					string value = line.Substring(startsWith.Length);

					if (value == "SET_CPU")
						return SubstanceEngineType.CPU;
					else if (value == "SET_GPU")
						return SubstanceEngineType.GPU;

					break;
				}
			}

			return SubstanceEngineType.CPU;
		}
		catch (Exception)
		{
			return SubstanceEngineType.CPU;
		}
	}

	public string GetBuildConfig(ReadOnlyTargetRules Target)
	{
		if (Target.Configuration == UnrealTargetConfiguration.Debug)
		{
			if (Target.IsInPlatformGroup(UnrealPlatformGroup.Windows))
			{
				if (Target.bDebugBuildsActuallyUseDebugCRT)
					return "Debug";
			}
			else
			{
				return "Debug";
			}
		}

		return "Release";
	}

	public string GetPlatformConfig(ReadOnlyTargetRules Target)
	{
		return Target.Platform.ToString();
	}
}
