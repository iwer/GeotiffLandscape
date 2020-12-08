// Copyright (c) Iwer Petersen. All rights reserved.

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "UnrealGDAL.h"

#define LOCTEXT_NAMESPACE "FGeotiffLandscapeModule"

class FGeotiffLandscapeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override
    {
        // Init GDAL module
        FUnrealGDALModule* UnrealGDAL = FModuleManager::Get().LoadModulePtr<FUnrealGDALModule>("UnrealGDAL");
        UnrealGDAL->InitGDAL();
    }
	virtual void ShutdownModule() override
    {}

    virtual bool SupportsDynamicReloading() override
    {return true;}
};


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGeotiffLandscapeModule, GeotiffLandscape)
