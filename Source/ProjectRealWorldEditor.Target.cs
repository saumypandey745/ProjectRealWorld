using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectRealWorldEditorTarget : TargetRules
{
	public ProjectRealWorldEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("ProjectRealWorld");
	}
}
