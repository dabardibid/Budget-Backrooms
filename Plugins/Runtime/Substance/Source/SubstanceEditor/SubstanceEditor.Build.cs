// Copyright 2017 Allegorithmic, Inc. All Rights Reserved.
using UnrealBuildTool;
using System;
using System.IO;
using System.Collections.Generic;

/**
 * Substance core is added as a private dependency as we do not want to expose substance core to other
 * modules that will use the substance editor module. We are still adding the framework/lib headers for our private includes
 * as substance core will depend on these and so that this module will know about the framework types.
 **/

public class SubstanceEditor : ModuleRules
{
    public SubstanceEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		//PCH file
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;
		PrivatePCHHeaderFile = "Private/SubstanceEditorPrivatePCH.h";

        //Include paths
        PrivateIncludePaths.Add("SubstanceEditor/Private");

        //Module public dependencies
        PublicDependencyModuleNames.AddRange(new string[]
		{
			"AssetTools",
			"BlueprintGraph",
			"ContentBrowser",
			"Core",
			"CoreUObject",
			"DesktopPlatform",
			"EditorStyle",
			"EditorWidgets",
			"Engine",
			"InputCore",
			"KismetCompiler",
			"LevelEditor",
			"MainFrame",
			"PropertyEditor",
			"RenderCore",
			"RHI",
			"TextureEditor",
			"UnrealEd",
		});

		//Module private dependencies
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AppFramework",
			"Slate",
			"SlateCore",
			"Projects",
			"WorkspaceMenuStructure",
			"SubstanceCore"
		});

	}
}
