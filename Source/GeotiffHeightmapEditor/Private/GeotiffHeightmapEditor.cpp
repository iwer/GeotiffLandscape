#include "Modules/ModuleManager.h"
#include "Toolkits/AssetEditorToolkit.h"

#include "AssetTools/GeotiffAssetActions.h"

class FGeotiffHeightmapEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		RegisterAssetTools();
    }

	virtual void ShutdownModule() override
	{
        UnregisterAssetTools();
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

        RegisterAssetTypeAction(AssetTools, MakeShareable(new FGeotiffAssetActions(Style.ToSharedRef())));
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

		if (AssetToolsModule != nullptr)
		{
			IAssetTools& AssetTools = AssetToolsModule->Get();

			for (auto Action : RegisteredAssetTypeActions)
			{
				AssetTools.UnregisterAssetTypeActions(Action);
			}
		}
	}
private:
    /** The collection of registered asset type actions. */
    TArray<TSharedRef<IAssetTypeActions>> RegisteredAssetTypeActions;
    /** Holds the plug-ins style set. */
    TSharedPtr<ISlateStyle> Style;
};
