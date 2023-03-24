// Copyright (c) Iwer Petersen. All rights reserved.
#include "GeotiffHeightmapAssetActions.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "GeotiffHeightmapAsset.h"
#include "Styling/SlateStyle.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FGeotiffHeightmapAssetActions::FGeotiffHeightmapAssetActions(const TSharedRef<ISlateStyle>& InStyle)
    : Style(InStyle)
{}

bool FGeotiffHeightmapAssetActions::CanFilter()
{
    return true;
}

void FGeotiffHeightmapAssetActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
    FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);

    auto TextAssets = GetTypedWeakObjectPtrs<UGeotiffHeightmapAsset>(InObjects);

    MenuBuilder.AddMenuEntry(
        LOCTEXT("GeotiffHeightmapAsset_DummyText", "Dummy Text"),
        LOCTEXT("GeotiffHeightmapAsset_DummyTextToolTip", "Dummy menu entry."),
        FSlateIcon(),
        FUIAction(
            FExecuteAction::CreateLambda([=]{

            }),
            FCanExecuteAction::CreateLambda([=] {
                return false;
            })
        )
    );
}

uint32 FGeotiffHeightmapAssetActions::GetCategories()
{
    return EAssetTypeCategories::Textures;
}


FText FGeotiffHeightmapAssetActions::GetName() const
{
    return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_GeotiffHeightmapAsset", "Geotiff Heightmap Asset");
}


UClass* FGeotiffHeightmapAssetActions::GetSupportedClass() const
{
    return UGeotiffHeightmapAsset::StaticClass();
}


FColor FGeotiffHeightmapAssetActions::GetTypeColor() const
{
    return FColor::Red;
}


bool FGeotiffHeightmapAssetActions::HasActions(const TArray<UObject*>& InObjects) const
{
    return true;
}



#undef LOCTEXT_NAMESPACE
