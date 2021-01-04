// Copyright (c) Iwer Petersen. All rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "LandscapeFileFormatInterface.h"

// Implement .tif file format
class FLandscapeHeightmapFileFormat_Geotiff : public ILandscapeHeightmapFileFormat
{
private:
        FLandscapeFileTypeInfo FileTypeInfo;

        FVector2D PredictLandscapeSize(int width, int height) const;
public:
        FLandscapeHeightmapFileFormat_Geotiff();

        virtual const FLandscapeFileTypeInfo& GetInfo() const override
        {
                return FileTypeInfo;
        }

        virtual FLandscapeHeightmapInfo Validate(const TCHAR* HeightmapFilename) const override;
        virtual FLandscapeHeightmapImportData Import(const TCHAR* HeightmapFilename, FLandscapeFileResolution ExpectedResolution) const override;
        virtual void Export(const TCHAR* HeightmapFilename, TArrayView<const uint16> Data, FLandscapeFileResolution DataResolution, FVector Scale) const override;
};


//////////////////////////////////////////////////////////////////////////

class FLandscapeWeightmapFileFormat_Geotiff : public ILandscapeWeightmapFileFormat
{
private:
        FLandscapeFileTypeInfo FileTypeInfo;

public:
        FLandscapeWeightmapFileFormat_Geotiff();

        virtual const FLandscapeFileTypeInfo& GetInfo() const override
        {
                return FileTypeInfo;
        }

        virtual FLandscapeWeightmapInfo Validate(const TCHAR* WeightmapFilename, FName LayerName) const override;
        virtual FLandscapeWeightmapImportData Import(const TCHAR* WeightmapFilename, FName LayerName, FLandscapeFileResolution ExpectedResolution) const override;
        virtual void Export(const TCHAR* WeightmapFilename, FName LayerName, TArrayView<const uint8> Data, FLandscapeFileResolution DataResolution) const override;
};
