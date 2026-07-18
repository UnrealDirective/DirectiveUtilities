using UnrealBuildTool;

public class DirectiveUtilitiesRuntimeHostEditorTarget : TargetRules
{
	public DirectiveUtilitiesRuntimeHostEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		bForceCompileDevelopmentAutomationTests = true;
		ExtraModuleNames.Add("DirectiveUtilitiesRuntimeHost");
	}
}
