#include "GeotiffAssetFactory.h"
#include "GeotiffFileHelper.h"
#include "GeotiffAsset.h"

UGeotiffAssetFactory::UGeotiffAssetFactory( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	Formats.Add(FString(TEXT("tif;")) + NSLOCTEXT("UGeotiffAssetFactory", "FormatTif", "Geotiff File").ToString());
	SupportedClass = UGeotiffAsset::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
}

UObject* UGeotiffAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	UGeotiffAsset* Asset = nullptr;
	float minHeight, maxHeight;
    UTexture2D * texture= NewObject<UTexture2D>(this);
    FGeoRegion region;

	if (GeotiffFileHelper::LoadFile(Filename, texture, region, minHeight, maxHeight))
	{
	    Asset = NewObject<UGeotiffAsset>(InParent, InClass, InName, Flags);
	    Asset->MinHeight = minHeight;
        Asset->MaxHeight = maxHeight;
        Asset->Texture = texture;
        Asset->GeoRegion = region;
	}

	bOutOperationCanceled = false;

	return Asset;
}
