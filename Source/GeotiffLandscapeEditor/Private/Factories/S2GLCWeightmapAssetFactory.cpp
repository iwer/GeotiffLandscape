// Copyright (c) Iwer Petersen. All rights reserved.
#include "S2GLCWeightmapAssetFactory.h"
#include "GeotiffRasterScaler.h"
#include "S2GLCWeightmapAsset.h"
#include "AssetRegistryModule.h"
#include "Engine/Texture2D.h"

US2GLCWeightmapAssetFactory::US2GLCWeightmapAssetFactory( const FObjectInitializer& ObjectInitializer )
    : Super(ObjectInitializer)
{
    SupportedClass = US2GLCWeightmapAsset::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
    Formats.Add(TEXT("s2glc;S2GLC Weightmap File"));
}


UObject* US2GLCWeightmapAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
    US2GLCWeightmapAsset* Asset = NewObject<US2GLCWeightmapAsset>(InParent, InClass, InName, Flags);


    // open file
    GDALDatasetRef gdaldata = GDALHelpers::OpenRaster(Filename, true);
    if(!gdaldata){
        bOutOperationCanceled = true;
        return nullptr;
    }

    Asset->ROI = NewObject<URegionOfInterest>(Asset);
    Asset->ROI->InitFromGDAL(gdaldata);

    int texsize = 2048;
    TArray<uint8> targetPixels;
    FString TextureName = TEXT("T_") + InName.ToString();

    const UEnum* classenum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ES2GLCClasses"), true);

    // only to NumEnums - 1 because enums contain an extra ENUMNAME_MAX value
    for(int i = 0; i < classenum->NumEnums()-1;i++){
        if(i == ES2GLCClasses::Clouds ||
            i == ES2GLCClasses::NoData) {
                Asset->LayerMaps.Add(nullptr);
                continue;
        }

        // create texture with name
        FName LayerName = classenum->GetNameByIndex(i);
        FString Name = TextureName + TEXT("_") + LayerName.ToString();
        UTexture2D * target = NewObject<UTexture2D>(Asset, *Name, RF_Public | RF_Standalone | RF_MarkAsRootSet);
        ExtractLayerMap(gdaldata, target, LayerName, targetPixels, texsize, texsize);
        Asset->LayerMaps.Add(target);
    }

    bOutOperationCanceled = false;


    return Asset;
}

bool US2GLCWeightmapAssetFactory::FactoryCanImport(const FString & Filename)
{
    const FString Extension = FPaths::GetExtension(Filename);

    if (Extension == TEXT("tif"))
    {
            return true;
    }
    return false;
}

void US2GLCWeightmapAssetFactory::ExtractLayerMap(GDALDatasetRef &Source, UTexture2D * TargetTexture, FName LayerName, TArray<uint8> Pixels, int Width, int Height)
{
    // Populate texture
    TargetTexture->AddToRoot();
    TargetTexture->PlatformData = new FTexturePlatformData();
    TargetTexture->PlatformData->SizeX = Width;
    TargetTexture->PlatformData->SizeY = Height;
    TargetTexture->PlatformData->PixelFormat = EPixelFormat::PF_G8;

    UGeotiffRasterScaler::ImportScaleGeotiffWeightLayer(Source, 1, LayerName, Pixels, Width, Height);

    FTexture2DMipMap* Mip = new FTexture2DMipMap();
    TargetTexture->PlatformData->Mips.Add(Mip);
    Mip->SizeX = Width;
    Mip->SizeY = Height;
    Mip->BulkData.Lock(LOCK_READ_WRITE);
    uint8* TextureData = (uint8*)Mip->BulkData.Realloc(sizeof(uint8) * Width * Height);
    FMemory::Memcpy(TextureData, Pixels.GetData(), sizeof(uint8) * Width * Height);
    Mip->BulkData.Unlock();

    TargetTexture->Source.Init(Width, Height, 1, 1, ETextureSourceFormat::TSF_G8, Pixels.GetData());
    TargetTexture->UpdateResource();

    FAssetRegistryModule::AssetCreated(TargetTexture);
}
