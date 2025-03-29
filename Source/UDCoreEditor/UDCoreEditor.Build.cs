using UnrealBuildTool;

public class UDCoreEditor : ModuleRules
{
	public UDCoreEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"EditorSubsystem",
				"UDCore",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"EditorScriptingUtilities",
				"UnrealEd",
			}
		);
	}
}