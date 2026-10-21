using UnrealBuildTool;

public class ProjectRealWorld : ModuleRules
{
	public ProjectRealWorld(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			// ── Core ──────────────────────────────────────────────────────────
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",

			// ── Input ─────────────────────────────────────────────────────────
			"EnhancedInput",

			// ── AI & Navigation ───────────────────────────────────────────────
			"AIModule",
			"NavigationSystem",
			"GameplayTasks",
			"StateTreeModule",
			"GameplayStateTree",

			// ── Physics & Vehicles ────────────────────────────────────────────
			"ChaosVehicles",
			"ChaosVehiclesCore",
			"PhysicsCore",
			"Chaos",

			// ── Audio ─────────────────────────────────────────────────────────
			"MetasoundEngine",
			"AudioExtensions",
			"AudioMixer",
			"SignalProcessing",

			// ── Animation ─────────────────────────────────────────────────────
			"MotionWarping",
			"ControlRig",
			"AnimGraphRuntime",

			// ── UI / HUD ──────────────────────────────────────────────────────
			"UMG",
			"Slate",
			"SlateCore",
			"CommonUI",

			// ── VFX ───────────────────────────────────────────────────────────
			"Niagara",
			"NiagaraCore",

			// ── Gameplay Framework ────────────────────────────────────────────
			"GameplayTags",
			"GameplayAbilities",

			// ── World Building ────────────────────────────────────────────────
			"PCG",
			"GeometryCore",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			// ── Rendering ─────────────────────────────────────────────────────
			"RenderCore",
			"RHI",
			"Renderer",

			// ── Net ───────────────────────────────────────────────────────────
			"NetCore",

			// ── Misc ──────────────────────────────────────────────────────────
			"DeveloperSettings",
		});

		PublicIncludePaths.AddRange(new string[] {
			"ProjectRealWorld"
		});

		// Disable PCH for faster iteration during development
		bUseUnity = false;
	}
}
