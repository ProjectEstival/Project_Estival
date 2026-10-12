using UnrealBuildTool;

public class DeveloperConsole : ModuleRules
{
    public DeveloperConsole(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
        PublicDependencyModuleNames.AddRange(new string[] { "Core" });
        PrivateDependencyModuleNames.AddRange(new string[]{
			"CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
            "InputCore",
		});
		
        // The plugin is compiled in all configurations, but its implementation is disabled in Shipping.
        if (Target.Configuration == UnrealTargetConfiguration.Shipping) PublicDefinitions.Add("WITH_DEVELOPER_CONSOLE=0");
        else PublicDefinitions.Add("WITH_DEVELOPER_CONSOLE=1");
    }
}