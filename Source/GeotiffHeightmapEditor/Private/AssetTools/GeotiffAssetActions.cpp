#include "GeotiffAssetActions.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "GeotiffAsset.h"
#include "Styling/SlateStyle.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

FGeotiffAssetActions::FGeotiffAssetActions(const TSharedRef<ISlateStyle>& InStyle)
	: Style(InStyle)
{}

bool FGeotiffAssetActions::CanFilter()
{
	return true;
}

void FGeotiffAssetActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);

	auto TextAssets = GetTypedWeakObjectPtrs<UGeotiffAsset>(InObjects);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("GeotiffAsset_DummyText", "Dummy Text"),
		LOCTEXT("GeotiffAsset_DummyTextToolTip", "Dummy menu entry."),
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

uint32 FGeotiffAssetActions::GetCategories()
{
	return EAssetTypeCategories::MaterialsAndTextures;
}


FText FGeotiffAssetActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_GeotiffAsset", "Geotiff Asset");
}


UClass* FGeotiffAssetActions::GetSupportedClass() const
{
	return UGeotiffAsset::StaticClass();
}


FColor FGeotiffAssetActions::GetTypeColor() const
{
	return FColor::White;
}


bool FGeotiffAssetActions::HasActions(const TArray<UObject*>& InObjects) const
{
	return true;
}



#undef LOCTEXT_NAMESPACE
