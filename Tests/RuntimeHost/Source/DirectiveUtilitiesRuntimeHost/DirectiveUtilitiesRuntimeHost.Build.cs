using UnrealBuildTool;

public class DirectiveUtilitiesRuntimeHost : ModuleRules
{
	public DirectiveUtilitiesRuntimeHost(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine"
			}
		);

		PrivateDependencyModuleNames.Add("DirectiveUtilitiesRuntime");
	}
}
