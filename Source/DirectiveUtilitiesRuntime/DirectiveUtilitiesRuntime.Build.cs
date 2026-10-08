// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

using UnrealBuildTool;

public class DirectiveUtilitiesRuntime : ModuleRules
{
	public DirectiveUtilitiesRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"NetCore",
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"AIModule",
				"NavigationSystem",
				"EnhancedInput",
				"ApplicationCore",
			}
		);

		AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
