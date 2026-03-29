// Copyright 2021 Sam Carey. All Rights Reserved.

using UnrealBuildTool;
 
public class AchievementSystemEditor : ModuleRules
{
	public AchievementSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "AchievementSystem", "UnrealEd", "Slate", "SlateCore", "PropertyEditor", "GraphEditor", "BlueprintGraph" });

		PublicDependencyModuleNames.AddRange(new string[] { "AchievementSystem", "KismetCompiler" });
 
		PublicIncludePaths.AddRange(new string[] {"AchievementSystemEditor/Public"});
		PrivateIncludePaths.AddRange(new string[] {"AchievementSystemEditor/Private"});
	}
}