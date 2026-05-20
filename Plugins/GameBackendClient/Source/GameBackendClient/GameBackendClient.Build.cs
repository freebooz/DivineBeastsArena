using UnrealBuildTool;

public class GameBackendClient : ModuleRules
{
	public GameBackendClient(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"HTTP",
			"Json",
			"JsonUtilities",
			"Projects",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});

		bUseUnity = false;
		CppStandard = CppStandardVersion.Cpp20;
	}
}
