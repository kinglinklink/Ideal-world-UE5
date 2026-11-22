using System.IO;
using UnrealBuildTool;

public class IdealWorld : ModuleRules
{
	public IdealWorld(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// The engine-free core (Core/) is plain C++17 and is also compiled on
		// its own by Tests/run_tests.sh, so it is unit-tested without Unreal.
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Core"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Game"));
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "UI"));

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"PhysicsCore",
			"ProceduralMeshComponent",
			"Slate",
			"SlateCore",
			"ImageWrapper",
			"RenderCore",
			"ApplicationCore",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Chaos",
			"Json",
		});
	}
}
