using UnrealBuildTool;

public class DirectorsCut_58EditorTarget : TargetRules
{
	public DirectorsCut_58EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("DirectorsCut_58");
	}
}
