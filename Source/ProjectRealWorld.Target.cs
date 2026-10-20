using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectRealWorldTarget : TargetRules
{
	public ProjectRealWorldTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("ProjectRealWorld");
	}
}
