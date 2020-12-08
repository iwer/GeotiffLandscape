// Copyright (c) Iwer Petersen. All rights reserved.
#include "S2GLCWeightmapAssetActions.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "S2GLCWeightmapAsset.h"
#include "Styling/SlateStyle.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FS2GLCWeightmapAssetActions::FS2GLCWeightmapAssetActions(const TSharedRef<ISlateStyle>& InStyle)
	: Style(InStyle)
{}

bool FS2GLCWeightmapAssetActions::CanFilter()
{
	return true;
}

void FS2GLCWeightmapAssetActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);

	auto TextAssets = GetTypedWeakObjectPtrs<US2GLCWeightmapAsset>(InObjects);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("S2GLCWeightmapAsset_DummyText", "Dummy Text"),
		LOCTEXT("S2GLCWeightmapAsset_DummyTextToolTip", "Dummy menu entry."),
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

uint32 FS2GLCWeightmapAssetActions::GetCategories()
{
	return EAssetTypeCategories::MaterialsAndTextures;
}


FText FS2GLCWeightmapAssetActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_S2GLCWeightmapAsset", "S2GLC Weightmap Asset");
}


UClass* FS2GLCWeightmapAssetActions::GetSupportedClass() const
{
	return US2GLCWeightmapAsset::StaticClass();
}


FColor FS2GLCWeightmapAssetActions::GetTypeColor() const
{
	return FColor::Green;
}


bool FS2GLCWeightmapAssetActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}



#undef LOCTEXT_NAMESPACE
