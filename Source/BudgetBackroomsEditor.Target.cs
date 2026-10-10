

using UnrealBuildTool;
using System.Collections.Generic;

public class BudgetBackroomsEditorTarget : TargetRules
{
	public BudgetBackroomsEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;

        ExtraModuleNames.AddRange( new string[] { "BudgetBackrooms" } );
	}
}
