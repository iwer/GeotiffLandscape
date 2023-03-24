// Copyright (c) Iwer Petersen. All rights reserved.


#include "LandscapeFileFormatGeoTiff.h"
#include "HAL/UnrealMemory.h"
#include "GeotiffRasterScaler.h"
#include "S2GLCClasses.h"
#include "GeoReferenceHelper.h"
#include "ROI.h"

#define LOCTEXT_NAMESPACE "LandscapeEditor.NewLandscape"

FLandscapeHeightmapFileFormat_Geotiff::FLandscapeHeightmapFileFormat_Geotiff()
{
    FileTypeInfo.Description = LOCTEXT("FileFormatGeotiff_HeightmapDesc", "Geotiff DEM Heightmap .tif files");
    FileTypeInfo.Extensions.Add(".tif");
    FileTypeInfo.bSupportsExport = false;
}

FLandscapeHeightmapInfo FLandscapeHeightmapFileFormat_Geotiff::Validate(const TCHAR* HeightmapFilename) const
{
    FLandscapeHeightmapInfo Result;

    GDALDatasetRef GdalData = GDALHelpers::OpenRaster(HeightmapFilename, true);
    if(!GdalData){
        // file open error
        Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_FileOpenError","Error opening file!");
        Result.ResultCode = ELandscapeImportResult::Error;
    } else {
        // get the first raster band
        const auto RasterBands = mergetiff::DatasetManagement::getRasterBands(GdalData, {1});

        // Check if grayscale
        if (strncmp(GDALGetColorInterpretationName(RasterBands[0]->GetColorInterpretation()), "Gray", 5) != 0) {
            // not a grayscale band
            Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_PixelTypeError","First Rasterband is not grayscale!");
            Result.ResultCode = ELandscapeImportResult::Error;
        } else {
            if((strncmp(GDALGetDataTypeName(RasterBands[0]->GetRasterDataType()), "Int16", 6) != 0) &&
                (strncmp(GDALGetDataTypeName(RasterBands[0]->GetRasterDataType()), "Float32", 8) != 0)) {
                // neither 16 bit int nor 32 bit float
                Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_BitDepthError","Pixelformat is not 'Int16' or 'Float32'!");
                Result.ResultCode = ELandscapeImportResult::Error;
            } else {
                auto Srs = GdalData->GetProjectionRef();
                double Width, Height;
                URegionOfInterest::GetSize(GdalData, Width, Height);
                FLandscapeFileResolution ImportResolution;
                ImportResolution.Width = Width;
                ImportResolution.Height = Height;

                const auto HeightMinMax = GDALHelpers::ComputeRasterMinMax(GdalData, 1);
                // auto PredMapSize = PredictLandscapeSize(width, height);
                Result.DataScale = FVector(100, //* width / PredMapSize.X,  // This scales the landscape to the correct size,
                                            100, //* height / PredMapSize.Y, // but also affect the mapping of the heightvalues to the landscape
                                            100.0/128 * UGeotiffRasterScaler::HeightScaleFactor(HeightMinMax->Max, HeightMinMax->Min));
                Result.PossibleResolutions.Add(ImportResolution);
                Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_ValidateSuccess","File validated");
                Result.ResultCode = ELandscapeImportResult::Success;
            }
        }
    }
    return Result;
}

FLandscapeHeightmapImportData FLandscapeHeightmapFileFormat_Geotiff::Import(const TCHAR* HeightmapFilename, FLandscapeFileResolution ExpectedResolution) const
{
    FLandscapeHeightmapImportData Result;

    GDALDatasetRef GdalData = GDALHelpers::OpenRaster(HeightmapFilename, true);

    if(!GdalData){
        // file open error
    } else {
        // get the first raster band
        GDALRasterBandH rasterBand = GdalData->GetRasterBand(1);

        const auto RasterBands = mergetiff::DatasetManagement::getRasterBands(GdalData, {1});

        double Width, Height;
        URegionOfInterest::GetSize(GdalData, Width, Height);

        // Check if grayscale
        if(strncmp(GDALGetColorInterpretationName(RasterBands[0]->GetColorInterpretation()), "Gray", 5) != 0) {
            // pixel type error

        } else
        {
            TArray<uint8> TempData;
            // Check pixel format
            if(strncmp(GDALGetDataTypeName(RasterBands[0]->GetRasterDataType()), "Int16", 6) == 0) {
                double HeightScale;
                UGeotiffRasterScaler::ConvertScaleGeotiffRaster<int16_t>(EPixelScanMode::G16_RG8, GdalData, 1, TempData, ExpectedResolution.Width, ExpectedResolution.Height, HeightScale);
                Result.Data.Empty(ExpectedResolution.Width * ExpectedResolution.Height);
                Result.Data.AddUninitialized(ExpectedResolution.Width * ExpectedResolution.Height);
                FMemory::Memcpy(Result.Data.GetData(), TempData.GetData(), ExpectedResolution.Width * ExpectedResolution.Height * 2);
            }
            else if (strncmp(GDALGetDataTypeName(RasterBands[0]->GetRasterDataType()), "Float32", 8) == 0) {
                double HeightScale;
                UGeotiffRasterScaler::ConvertScaleGeotiffRaster<float>(EPixelScanMode::F32_RG8, GdalData, 1, TempData, ExpectedResolution.Width, ExpectedResolution.Height, HeightScale);
                Result.Data.Empty(ExpectedResolution.Width * ExpectedResolution.Height);
                Result.Data.AddUninitialized(ExpectedResolution.Width * ExpectedResolution.Height);
                FMemory::Memcpy(Result.Data.GetData(), TempData.GetData(), ExpectedResolution.Width * ExpectedResolution.Height * 2);
            }
        }
    }
    return Result;
}

void FLandscapeHeightmapFileFormat_Geotiff::Export(const TCHAR* HeightmapFilename, TArrayView<const uint16> Data, FLandscapeFileResolution DataResolution, FVector Scale) const
{

}

//////////////////////////////////////////////////////////////////////////

FLandscapeWeightmapFileFormat_Geotiff::FLandscapeWeightmapFileFormat_Geotiff()
{
        FileTypeInfo.Description = LOCTEXT("FileFormatGeotiff_WeightmapDesc", "Geotiff Weightmap .tif files");
        FileTypeInfo.Extensions.Add(".tif");
        FileTypeInfo.bSupportsExport = false;
}

FLandscapeWeightmapInfo FLandscapeWeightmapFileFormat_Geotiff::Validate(const TCHAR* WeightmapFilename, FName LayerName) const
{
    FLandscapeWeightmapInfo Result;

    // Open the file
    GDALDatasetRef GdalData = GDALHelpers::OpenRaster(WeightmapFilename, true);

    if(!GdalData){
        // file open error
        Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_FileOpenError","Error opening file!");
        Result.ResultCode = ELandscapeImportResult::Error;
    } else {
        // get the first raster band
        const GDALRasterBandH RasterBand = GdalData->GetRasterBand(1);

        auto rasterBands = mergetiff::DatasetManagement::getRasterBands(GdalData, {1});
        // Check if grayscale
        if (strncmp(GDALGetColorInterpretationName(GDALGetRasterColorInterpretation(RasterBand)), "Gray", 5) != 0) {
            // not a grayscale band
            Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_PixelTypeError","First Rasterband is not grayscale!");
            Result.ResultCode = ELandscapeImportResult::Error;
        } else {
            if(strncmp(GDALGetDataTypeName(GDALGetRasterDataType(RasterBand)), "Byte", 5) != 0) {
                // not 16 bit
                Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_BitDepthError","Pixelformat is not 'Byte'!");
                Result.ResultCode = ELandscapeImportResult::Error;
            } else {
                const auto Srs = GdalData->GetProjectionRef();
                if(!FGeoReferenceHelper::IsWGS84(OSRNewSpatialReference(Srs)) &&
                   !FGeoReferenceHelper::IsUTM(OSRNewSpatialReference(Srs))){
                    Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_SpatialReferenceError","Spatial Reference not supported!");
                    Result.ResultCode = ELandscapeImportResult::Error;
                } else {
                    double Width, Height;
                    URegionOfInterest::GetSize(GdalData, Width, Height);
                    FLandscapeFileResolution ImportResolution;
                    ImportResolution.Width = Width;
                    ImportResolution.Height = Height;
                    Result.PossibleResolutions.Add(ImportResolution);
                    Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_ValidateSuccess","File validated");
                    Result.ResultCode = ELandscapeImportResult::Success;
                }
            }
        }
    }
    return Result;
}

FLandscapeWeightmapImportData FLandscapeWeightmapFileFormat_Geotiff::Import(const TCHAR* WeightmapFilename, FName LayerName, FLandscapeFileResolution ExpectedResolution) const
{
    FLandscapeWeightmapImportData Result;

    // Open the file
    GDALDatasetRef GdalData = GDALHelpers::OpenRaster(WeightmapFilename, true);
    if(!GdalData){
        // file open error
    } else {
        // get the first raster band
        //GDALRasterBandH  rasterBand = gdaldata->GetRasterBand(1);
        const auto RasterBands = mergetiff::DatasetManagement::getRasterBands(GdalData, {1});

        // Check if grayscale
        if(strncmp(GDALGetColorInterpretationName(RasterBands[0]->GetColorInterpretation()), "Gray", 5) != 0) {
            // pixel type error

        } else {
            // Check pixel format
            if(strncmp(GDALGetDataTypeName(RasterBands[0]->GetRasterDataType()), "Byte", 5) != 0) {

            } else {
                TArray<uint8> TempData;
                double HeightScale;
                UGeotiffRasterScaler::ConvertScaleGeotiffRaster<uint8_t>(EPixelScanMode::G8_G8, GdalData, 1, TempData, ExpectedResolution.Width, ExpectedResolution.Height, HeightScale);
                // if(!UGeotiffRasterScaler::ImportScaleGeotiffWeightLayer(gdaldata, 1, LayerName, TempData, ExpectedResolution.Width, ExpectedResolution.Height)){
                //     Result.ResultCode = ELandscapeImportResult::Error;
                //     Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_ImportFailure", "Weightmap import failed!");
                // } else {
                    Result.Data.Empty(ExpectedResolution.Width * ExpectedResolution.Height);
                    Result.Data.AddUninitialized(ExpectedResolution.Width * ExpectedResolution.Height);
                    FMemory::Memcpy(Result.Data.GetData(), TempData.GetData(), ExpectedResolution.Width * ExpectedResolution.Height);
                // }
            }
        }
    }

    return Result;
}

void FLandscapeWeightmapFileFormat_Geotiff::Export(const TCHAR* WeightmapFilename, FName LayerName, TArrayView<const uint8> Data, FLandscapeFileResolution DataResolution) const
{

}

#undef LOCTEXT_NAMESPACE
