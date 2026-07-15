// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

using UnrealBuildTool;

public class DirectiveUtilitiesBlueprintNodes : ModuleRules
{
	public DirectiveUtilitiesBlueprintNodes(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"BlueprintGraph",
				"Core",
				"CoreUObject",
				"Engine",
				"DirectiveUtilitiesRuntime",
			}
		);

		PrivateDependencyModuleNames.Add("UnrealEd");
	}
}
