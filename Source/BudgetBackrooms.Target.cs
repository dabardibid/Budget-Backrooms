

using UnrealBuildTool;
using System.Collections.Generic;

public class BudgetBackroomsTarget : TargetRules
{
	public BudgetBackroomsTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;

        ExtraModuleNames.AddRange( new string[] { "BudgetBackrooms" } );
        bUsesSteam = true;
    }
}
