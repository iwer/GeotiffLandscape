// Copyright (c) Iwer Petersen. All rights reserved.
#include "Modules/ModuleManager.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "LandscapeEditorModule.h"

#include "AssetTools/GeotiffHeightmapAssetActions.h"
#include "AssetTools/S2GLCWeightmapAssetActions.h"
#include "Landscape/LandscapeFileFormatGeoTiff.h"
#include "Landscape/LandscapeFileFormatS2GLC.h"
#include "Styles/GeotiffHeightmapAssetEditorStyle.h"

#define LOCTEXT_NAMESPACE "FGeotiffLandscapeEditorModule"

class FGeotiffLandscapeEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
        Style = MakeShareable(new FGeotiffHeightmapAssetEditorStyle());

		RegisterAssetTools();
        RegisterLandscapeFileFormats();
    }

	virtual void ShutdownModule() override
	{
        UnregisterAssetTools();
        //UnregisterLandscapeFileFormats();
	}

    virtual bool SupportsDynamicReloading() override
	{
		return true;
	}
protected:
    /** Registers asset tool actions. */
    void RegisterAssetTools()
    {
        IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
        TSharedPtr<IAssetTypeActions> heightActionType = MakeShareable(new FGeotiffHeightmapAssetActions(Style.ToSharedRef()));
        RegisterAssetTypeAction(AssetTools, heightActionType.ToSharedRef());
        TSharedPtr<IAssetTypeActions> weightActionType = MakeShareable(new FS2GLCWeightmapAssetActions(Style.ToSharedRef()));
        RegisterAssetTypeAction(AssetTools, weightActionType.ToSharedRef());

    }

    /**
	 * Registers a single asset type action.
	 *
	 * @param AssetTools The asset tools object to register with.
	 * @param Action The asset type action to register.
	 */
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		RegisteredAssetTypeActions.Add(Action);
	}

    /** Unregisters asset tool actions. */
	void UnregisterAssetTools()
	{
		FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

		if (AssetToolsModule)
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (auto Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}
	}

    void RegisterLandscapeFileFormats()
    {
        ILandscapeEditorModule * LandscapeModule = FModuleManager::LoadModulePtr<ILandscapeEditorModule>("LandscapeEditor");

        if(LandscapeModule) {
            LandscapeModule->RegisterHeightmapFileFormat(MakeShareable(new FLandscapeHeightmapFileFormat_Geotiff()));
            LandscapeModule->RegisterWeightmapFileFormat(MakeShareable(new FLandscapeWeightmapFileFormat_Geotiff()));
            LandscapeModule->RegisterWeightmapFileFormat(MakeShareable(new FLandscapeWeightmapFileFormat_S2GLC()));
        } else {
             UE_LOG(LogTemp, Warning, TEXT("Could not get Landscape Editor Module to register geotiff landscape filetypes"));
        }

    }


private:
    /** The collection of registered asset type actions. */
    TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
    /** Holds the plug-ins style set. */
    TSharedPtr<ISlateStyle> Style;
};

IMPLEMENT_MODULE(FGeotiffLandscapeEditorModule, GeotiffLandscapeEditor);
#undef LOCTEXT_NAMESPACE
