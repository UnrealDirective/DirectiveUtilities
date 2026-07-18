using UnrealBuildTool;

public class DirectiveUtilitiesRuntimeHostTarget : TargetRules
{
	public DirectiveUtilitiesRuntimeHostTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		bBuildDeveloperTools = true;
		bForceCompileDevelopmentAutomationTests = true;
		ExtraModuleNames.AddRange(
			new string[]
			{
				"DirectiveUtilitiesRuntimeHost",
				"DirectiveUtilitiesRuntimeHostTests"
			}
		);
	}
}
