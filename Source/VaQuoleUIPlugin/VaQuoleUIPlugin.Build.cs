// Copyright 2014 Vladimir Alyamkin. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class VaQuoleUIPlugin : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }
    private string PluginPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../")); }
    }
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }
    public VaQuoleUIPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivateIncludePaths.AddRange(
				new string[] {
					"VaQuoleUIPlugin/Private",
				});

	PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
                    "InputCore",
                    "RenderCore",
					"RHI"
				});

            LoadVaQuoleDLL();
    }

    public bool LoadVaQuoleDLL()
    {
        bool IsLibrarySupported = false;
        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            IsLibrarySupported = true;

            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "Win64" : "Win32";
            string LibrariesPath = Path.Combine(ThirdPartyPath, "VaQuoleUI", "Lib", PlatformString);

            // Qt dependency libraries
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "Qt5Core.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "Qt5Gui.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "Qt5Network.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "Qt5Widgets.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "Qt5WebKit.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "Qt5WebKitWidgets.lib"));

            // VaQuoleLib
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "VaQuoleUILib.lib"));
        }

        if (IsLibrarySupported)
        {
            PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "VaQuoleUI", "Include"));
        }

        PublicDefinitions.Add(string.Format("WITH_VAQUOLE={0}", IsLibrarySupported ? 1 : 0));

        return IsLibrarySupported;
    }
}
