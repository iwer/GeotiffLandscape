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
    static bool VerifyS2GLCColors(GDALDatasetRef &dataset) {
		auto rasterdata = mergetiff::DatasetManagement::rasterFromDataset<uint8_t>(dataset, { static_cast<unsigned int>(1) });
		auto rasterBands = mergetiff::DatasetManagement::getRasterBands(dataset, { 1 });

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
		TSet<uint8_t> pixelValues;
		pixelValues.Empty();
		for (int y = 0; y < rasterBands[0]->GetYSize(); y++) {
			for (int x = 0; x < rasterBands[0]->GetXSize(); x++) {
				auto pixel = rasterdata.pixelComponent(y, x, 0);
				//UE_LOG(LogTemp, Warning, TEXT("Pixel: %d"), pixel);
				pixelValues.Add(pixel);
				if(!S2GLCColors.Contains(pixel))
					UE_LOG(LogTemp, Warning, TEXT("Pixel: %d %dx%d"), pixel, x, y);
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("pixelValues Num: %d"), pixelValues.Num());

		if (S2GLCColors.Includes(pixelValues))
			return true;

		return false;
    }
};
