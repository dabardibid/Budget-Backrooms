using UnrealBuildTool;
using System;
using System.Diagnostics;
using System.IO;




public class BudgetBackrooms : ModuleRules
{
	public BudgetBackrooms(ReadOnlyTargetRules Target) : base(Target)
    {


        // THANK YOU LORENZOHAPPY19 VERY DEMURE FOR THE GIT HASH THING THANK YOU VERY MUCH GRAZIE CAPAREZZA CALABRESE CARBONARA PERFETTA MILLE GRAZIE 
        var proc = new Process();
        var procf = new Process();
        proc.StartInfo.FileName = "git";
        proc.StartInfo.Arguments = "rev-parse --short HEAD";
        proc.StartInfo.RedirectStandardOutput = true;
        proc.StartInfo.UseShellExecute = false;
        proc.StartInfo.WorkingDirectory = Target.ProjectFile.Directory.FullName;
        procf.StartInfo.FileName = "git";
        procf.StartInfo.Arguments = "rev-parse HEAD";
        procf.StartInfo.RedirectStandardOutput = true;
        procf.StartInfo.UseShellExecute = false;
        procf.StartInfo.WorkingDirectory = Target.ProjectFile.Directory.FullName;
        proc.Start();
        procf.Start();
        string hash = proc.StandardOutput.ReadToEnd().Trim();
        string hashf = procf.StandardOutput.ReadToEnd().Trim();
        proc.WaitForExit();
        procf.WaitForExit();
        string header =
            "#pragma once\n" +
            "#define PROJECT_GIT_HASH \"" + hash + "\"\n" +
            "#define PROJECT_GIT_HASH_LONG \"" + hashf + "\"\n";
        string outPath = Path.Combine(ModuleDirectory, "BBHashThing.h");
        File.WriteAllText(outPath, header);


        RuntimeDependencies.Add(
            Path.Combine("$(ProjectDir)", "Config", "DefaultInput.ini"),
            StagedFileType.NonUFS
        );


        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG" });

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore", "RHI" });

        // Uncomment if you are using online features
        PublicDependencyModuleNames.AddRange(new string[] { "OnlineSubsystem", "OnlineSubsystemSteam", "Steamworks" });

        PublicDependencyModuleNames.AddRange(new string[] { "PakFile", "Projects" });


        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

        PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS"); // oh no unsafe code???

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicSystemLibraries.Add("dxgi.lib");
        }
    }
}
