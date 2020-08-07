// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "gdal_priv.h"
#include "cpl_conv.h"

class FGeotiffHeightmapModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override
    {
        // Set GDAL data dir and init drivers
        FString dataDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries"), TEXT("Data"), TEXT("GDAL")));
        CPLSetConfigOption("GDAL_DATA", TCHAR_TO_UTF8(*dataDir));

        //Register all GDAL format drivers
        GDALAllRegister();
    }
	virtual void ShutdownModule() override
    {}

    virtual bool SupportsDynamicReloading() override
    {return true;}
};
#define LOCTEXT_NAMESPACE "FGeotiffHeightmapModule"


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGeotiffHeightmapModule, GeotiffHeightmap)
