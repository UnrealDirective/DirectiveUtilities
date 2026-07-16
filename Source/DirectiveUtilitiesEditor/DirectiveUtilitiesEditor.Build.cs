// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

using UnrealBuildTool;

public class DirectiveUtilitiesEditor : ModuleRules
{
	public DirectiveUtilitiesEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"EditorSubsystem",
				"EditorScriptingUtilities",
				"AssetRegistry",
				"DirectiveUtilitiesRuntime",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"UnrealEd",
				"AssetTools",
			}
		);
	}
}
