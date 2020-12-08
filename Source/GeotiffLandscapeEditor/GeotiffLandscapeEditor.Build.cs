// Copyright (c) Iwer Petersen. All rights reserved.

using UnrealBuildTool;

public class GeotiffLandscapeEditor : ModuleRules
{
	public GeotiffLandscapeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			});


		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				"GeotiffLandscapeEditor/Private",
				"GeotiffLandscapeEditor/Private/Factories",
				"GeotiffLandscapeEditor/Private/Helpers",
				"GeotiffLandscapeEditor/Private/AssetTools",
				"GeotiffLandscapeEditor/Private/Landscape",
				"GeotiffLandscapeEditor/Private/Styles"
			});


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...
				"ContentBrowser",
				"Core",
				"CoreUObject",
				"DesktopWidgets",
				"EditorStyle",
				"Engine",
				"InputCore",
				"Projects",
                "RenderCore",
				"Slate",
				"SlateCore",
				"UnrealEd",
                "LandscapeEditor",
                "GDAL",
                "UnrealGDAL",
                "GeoReference",
                "GeotiffLandscape"
			});


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
                "AssetTools",
				"MainFrame"
			});

	    PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"AssetTools",
				"UnrealEd"
			});
	}
}
