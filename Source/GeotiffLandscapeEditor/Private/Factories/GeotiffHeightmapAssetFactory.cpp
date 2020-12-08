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
    auto band = gdaldata->GetRasterBand(1);


    Asset->MinHeight = band->GetMinimum();
    Asset->MaxHeight = band->GetMaximum();

	Asset->ROI = NewObject<URegionOfInterest>(Asset);
	Asset->ROI->InitFromGDAL(gdaldata);
	
    int texsize = 2048;
    FString TextureName = TEXT("T_") + InName.ToString();

    // Create texture
    Asset->Texture = NewObject<UTexture2D>(Asset, *TextureName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
    Asset->Texture->AddToRoot();
    Asset->Texture->PlatformData = new FTexturePlatformData();
    Asset->Texture->PlatformData->SizeX = texsize;
    Asset->Texture->PlatformData->SizeY = texsize;
    Asset->Texture->PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;

    // Pixel Memory
    TArray<uint8> targetPixels;
    double ScaleSuggestion;

	auto datatype = band->GetRasterDataType();
	if (datatype == GDT_Int16) {
		UGeotiffRasterScaler::ConvertScaleGeotiffRaster<int16_t>(EPixelScanMode::G16_BGRA8, gdaldata, 1, targetPixels, texsize, texsize, ScaleSuggestion);
	}
	else if (datatype == GDT_Float32) {
		UGeotiffRasterScaler::ConvertScaleGeotiffRaster<float>(EPixelScanMode::F32_BGRA8, gdaldata, 1, targetPixels, texsize, texsize, ScaleSuggestion);
	}

    FTexture2DMipMap* Mip = new FTexture2DMipMap();
    Asset->Texture->PlatformData->Mips.Add(Mip);
    Mip->SizeX = texsize;
    Mip->SizeY = texsize;
    Mip->BulkData.Lock(LOCK_READ_WRITE);
    uint8* TextureData = (uint8*)Mip->BulkData.Realloc(sizeof(uint8) * texsize * texsize * 4);
    FMemory::Memcpy(TextureData, targetPixels.GetData(), sizeof(uint8) * texsize * texsize * 4);
    Mip->BulkData.Unlock();

    Asset->Texture->Source.Init(texsize, texsize, 1, 1, ETextureSourceFormat::TSF_BGRA8, targetPixels.GetData());
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
