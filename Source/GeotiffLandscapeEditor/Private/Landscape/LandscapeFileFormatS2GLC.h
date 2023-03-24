// Copyright (c) Iwer Petersen. All rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "LandscapeFileFormatInterface.h"

// Implement .tif file format
class FLandscapeWeightmapFileFormat_S2GLC : public ILandscapeWeightmapFileFormat
{
private:
        FLandscapeFileTypeInfo FileTypeInfo;

public:
        FLandscapeWeightmapFileFormat_S2GLC();

        virtual const FLandscapeFileTypeInfo& GetInfo() const override
        {
                return FileTypeInfo;
        }

        virtual FLandscapeWeightmapInfo Validate(const TCHAR* WeightmapFilename, FName LayerName) const override;
        virtual FLandscapeWeightmapImportData Import(const TCHAR* WeightmapFilename, FName LayerName, FLandscapeFileResolution ExpectedResolution) const override;
        virtual void Export(const TCHAR* WeightmapFilename, FName LayerName, TArrayView<const uint8> Data, FLandscapeFileResolution DataResolution, FVector Scale) const override;
};
