// Copyright (c) Iwer Petersen. All rights reserved.
#include "GeotiffHeightmapAssetFactoryNew.h"

#include "GeotiffHeightmapAsset.h"

UGeotiffHeightmapAssetFactoryNew::UGeotiffHeightmapAssetFactoryNew(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SupportedClass = UGeotiffHeightmapAsset::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
    Formats.Add(TEXT("tif;Geotiff Heightmap File"));
}

UObject* UGeotiffHeightmapAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<UGeotiffHeightmapAsset>(InParent, InClass, InName, Flags);
}


bool UGeotiffHeightmapAssetFactoryNew::ShouldShowInNewMenu() const
{
    return true;
}
