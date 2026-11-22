using UnrealBuildTool;
using System.Collections.Generic;

public class IdealWorldTarget : TargetRules
{
	public IdealWorldTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("IdealWorld");
	}
}
