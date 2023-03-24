// Copyright (c) Iwer Petersen. All rights reserved.
#include "GeotiffHeightmapAssetFactory.h"
#include "GeotiffRasterScaler.h"
#include "GeotiffHeightmapAsset.h"
#include "AssetRegistryModule.h"

UGeotiffHeightmapAssetFactory::UGeotiffHeightmapAssetFactory( const FObjectInitializer& ObjectInitializer )
    : Super(ObjectInitializer)
{
    SupportedClass = UGeotiffHeightmapAsset::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
    Formats.Add(TEXT("tif;Geotiff Heightmap File"));
}

UObject* UGeotiffHeightmapAssetFactory::FactoryCreateFile(UClass* InClass,
                                                          UObject* InParent,
                                                          FName InName,
                                                          EObjectFlags Flags,
                                                          const FString& Filename,
                                                          const TCHAR* Parms,
                                                          FFeedbackContext* Warn,
                                                          bool& bOutOperationCanceled)
{
    UGeotiffHeightmapAsset* Asset = NewObject<UGeotiffHeightmapAsset>(InParent, InClass, InName, Flags);

    // open file
    GDALDatasetRef gdaldata = GDALHelpers::OpenRaster(Filename, true);
    if(!gdaldata){
        bOutOperationCanceled = true;
        return nullptr;
    }

    // Min Max Heights, as present in file, do not recompute
    //RasterMinMaxRef rasterheights = GDALHelpers::ComputeRasterMinMax(gdaldata, 1);
    const auto Band = gdaldata->GetRasterBand(1);


    Asset->MinHeight = Band->GetMinimum();
    Asset->MaxHeight = Band->GetMaximum();

    Asset->Roi = NewObject<URegionOfInterest>(Asset);
    Asset->Roi->InitFromGDAL(gdaldata);

    constexpr int TexSize = 2048;
    const FString TextureName = TEXT("T_") + InName.ToString();

    // Create texture
    Asset->Texture = NewObject<UTexture2D>(Asset, *TextureName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
    Asset->Texture->AddToRoot();
    Asset->Texture->PlatformData = new FTexturePlatformData();
    Asset->Texture->PlatformData->SizeX = TexSize;
    Asset->Texture->PlatformData->SizeY = TexSize;
    Asset->Texture->PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;

    // Pixel Memory
    TArray<uint8> TargetPixels;
    double ScaleSuggestion;

    const auto Datatype = Band->GetRasterDataType();
    if (Datatype == GDT_Int16) {
        UGeotiffRasterScaler::ConvertScaleGeotiffRaster<int16_t>(EPixelScanMode::G16_BGRA8, gdaldata, 1, TargetPixels, TexSize, TexSize, ScaleSuggestion);
    }
    else if (Datatype == GDT_Float32) {
        UGeotiffRasterScaler::ConvertScaleGeotiffRaster<float>(EPixelScanMode::F32_BGRA8, gdaldata, 1, TargetPixels, TexSize, TexSize, ScaleSuggestion);
    }

    FTexture2DMipMap* Mip = new FTexture2DMipMap();
    Asset->Texture->PlatformData->Mips.Add(Mip);
    Mip->SizeX = TexSize;
    Mip->SizeY = TexSize;
    Mip->BulkData.Lock(LOCK_READ_WRITE);
    uint8* TextureData = static_cast<uint8*>(Mip->BulkData.Realloc(sizeof(uint8) * TexSize * TexSize * 4));
    FMemory::Memcpy(TextureData, TargetPixels.GetData(), sizeof(uint8) * TexSize * TexSize * 4);
    Mip->BulkData.Unlock();

    Asset->Texture->Source.Init(TexSize, TexSize, 1, 1, ETextureSourceFormat::TSF_BGRA8, TargetPixels.GetData());
    Asset->Texture->UpdateResource();

    FAssetRegistryModule::AssetCreated(Asset->Texture);

    Asset->MarkPackageDirty();

    bOutOperationCanceled = false;


    return Asset;
}

bool UGeotiffHeightmapAssetFactory::FactoryCanImport(const FString & Filename)
{
    const FString Extension = FPaths::GetExtension(Filename);

    if (Extension == TEXT("tif"))
    {
            return true;
    }
    return false;
}
