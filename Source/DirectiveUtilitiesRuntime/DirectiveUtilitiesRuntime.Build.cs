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
				"GameplayTags",
				"NetCore",
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"AIModule",
				"EnhancedInput",
				"ApplicationCore",
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}