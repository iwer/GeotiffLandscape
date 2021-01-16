// Copyright (c) Iwer Petersen. All rights reserved.

using UnrealBuildTool;

public class GeotiffLandscape : ModuleRules
{
    public GeotiffLandscape(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
            });

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "GDAL",
                "UnrealGDAL",
                "GeoReference",
                "Projects"
                // ... add other public dependencies that you statically link with here ...
            });


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                // ... add private dependencies that you statically link with here ...
                "Engine",
                "CoreUObject",
                "ImageCore",
                "ImageWrapper"
            });


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            });
    }
}
