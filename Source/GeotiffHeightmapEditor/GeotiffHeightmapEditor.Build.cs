// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GeotiffHeightmapEditor : ModuleRules
{
	public GeotiffHeightmapEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);


		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				"GeotiffHeightmapEditor/Private",
				"GeotiffHeightmapEditor/Private/Factories",
                "GeotiffHeightmapEditor/Private/Helpers",
                "GeotiffHeightmapEditor/Private/AssetTools"
			}
			);


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
				"GeotiffHeightmap",
				"UnrealEd"
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
                "AssetTools",
				"MainFrame"
			}
			);

	    PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"AssetTools",
				"UnrealEd"
			});
	}
}
