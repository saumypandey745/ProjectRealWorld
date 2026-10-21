using UnrealBuildTool;

public class ProjectRealWorld : ModuleRules
{
	public ProjectRealWorld(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"ChaosVehicles",
			"PhysicsCore",
			"MetasoundEngine",
			"AudioExtensions",
			"MotionWarping",
			"UMG",
			"Slate",
			"SlateCore",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"RenderCore",
			"RHI"
		});

		PublicIncludePaths.AddRange(new string[] {
			"ProjectRealWorld"
		});
	}
}
