using UnrealBuildTool;

public class DirectiveUtilitiesRuntimeHostTests : ModuleRules
{
	public DirectiveUtilitiesRuntimeHostTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AutomationTest",
				"EnhancedInput",
				"GameplayTags",
				"DirectiveUtilitiesRuntime"
			}
		);
	}
}
