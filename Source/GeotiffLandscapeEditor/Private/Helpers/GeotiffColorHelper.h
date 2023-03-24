// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once

#include "S2GLCClasses.h"
#include "UObject/Class.h"
#include "GDALHelpers.h"
#include "GeotiffColorHelper.generated.h"

UCLASS()
class UGeotiffColorHelper
    : public UObject
{
    GENERATED_BODY()
public:
    static bool VerifyS2GLCColors(GDALDatasetRef &Dataset) {
        auto RasterData = mergetiff::DatasetManagement::rasterFromDataset<uint8_t>(Dataset, { static_cast<unsigned int>(1) });
        const auto RasterBands = mergetiff::DatasetManagement::getRasterBands(Dataset, { 1 });

        // Make Set os S2GLC colors
        TSet<uint8_t> S2GLCColors;
        for (int j = 0; j < FS2GLCColors::ClassCount; j++) {
            S2GLCColors.Add(FS2GLCColors::TifCol[j]);
        }

        for (auto &c : S2GLCColors) {
            UE_LOG(LogTemp, Warning, TEXT("S2GLCColor: %d"), c);
        }
        UE_LOG(LogTemp, Warning, TEXT("S2GLCColors Num: %d"), S2GLCColors.Num());

        // collect unique pixel values in dataset
        TSet<uint8_t> PixelValues;
        PixelValues.Empty();
        for (int y = 0; y < RasterBands[0]->GetYSize(); y++) {
            for (int x = 0; x < RasterBands[0]->GetXSize(); x++) {
                auto Pixel = RasterData.pixelComponent(y, x, 0);
                //UE_LOG(LogTemp, Warning, TEXT("Pixel: %d"), pixel);
                PixelValues.Add(Pixel);
                if(!S2GLCColors.Contains(Pixel))
                    UE_LOG(LogTemp, Warning, TEXT("Pixel: %d %dx%d"), Pixel, x, y);
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("pixelValues Num: %d"), PixelValues.Num());

        if (S2GLCColors.Includes(PixelValues))
            return true;

        return false;
    }
};
