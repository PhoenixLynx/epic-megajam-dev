using UnrealBuildTool;

public class DirectorsCut_58 : ModuleRules
{
	public DirectorsCut_58(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore",
			"AIModule", "NavigationSystem", "PhysicsCore", "UMG", "Slate", "SlateCore",
			"ProceduralMeshComponent"   // the cake (TakeCake): a real cake you can cut
		});
	}
}
