// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GeotiffHeightmap : ModuleRules
{
	public GeotiffHeightmap(ReadOnlyTargetRules Target) : base(Target)
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
                "Runtime/GeotiffHeightmap/Private"
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GDAL",
				"Projects"
				// ... add other public dependencies that you statically link with here ...
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...
				"Engine",
				"CoreUObject",
				"ImageCore",
				"ImageWrapper"
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
