using UnrealBuildTool;

public class DirectorsCut_58Target : TargetRules
{
	public DirectorsCut_58Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("DirectorsCut_58");
	}
}
