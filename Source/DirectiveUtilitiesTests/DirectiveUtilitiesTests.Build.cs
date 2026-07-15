// Copyright (c) 2026 Unreal Directive. Licensed under the MIT License.

using UnrealBuildTool;

public class DirectiveUtilitiesTests : ModuleRules
{
    public DirectiveUtilitiesTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "DirectiveUtilitiesRuntime",
                "AutomationTest",
                "EnhancedInput",
                "GameplayTags"
            }
        );

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("BlueprintGraph");
            PrivateDependencyModuleNames.Add("DirectiveUtilitiesBlueprintNodes");
            PrivateDependencyModuleNames.Add("DirectiveUtilitiesEditor");
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}
