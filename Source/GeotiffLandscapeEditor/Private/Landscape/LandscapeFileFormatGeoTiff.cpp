// Copyright (c) Iwer Petersen. All rights reserved.


#include "LandscapeFileFormatGeoTiff.h"
#include "HAL/UnrealMemory.h"
#include "GeotiffRasterScaler.h"
#include "S2GLCClasses.h"
#include "GeoReference.h"

#define LOCTEXT_NAMESPACE "LandscapeEditor.NewLandscape"

FLandscapeHeightmapFileFormat_Geotiff::FLandscapeHeightmapFileFormat_Geotiff()
{
    FileTypeInfo.Description = LOCTEXT("FileFormatGeotiff_HeightmapDesc", "DEM Heightmap .tif files");
    FileTypeInfo.Extensions.Add(".tif");
    FileTypeInfo.bSupportsExport = false;


}

FLandscapeHeightmapInfo FLandscapeHeightmapFileFormat_Geotiff::Validate(const TCHAR* HeightmapFilename) const
{
    FLandscapeHeightmapInfo Result;

    GDALDatasetRef gdaldata = GDALHelpers::OpenRaster(HeightmapFilename, true);
    if(!gdaldata){
        // file open error
        Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_FileOpenError","Error opening file!");
        Result.ResultCode = ELandscapeImportResult::Error;
    } else {
        // get the first raster band
        auto rasterBands = mergetiff::DatasetManagement::getRasterBands(gdaldata, {1});

        // Check if grayscale
        if (strncmp(GDALGetColorInterpretationName(rasterBands[0]->GetColorInterpretation()), "Gray", 5) != 0) {
            // not a grayscale band
            Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_PixelTypeError","First Rasterband is not grayscale!");
            Result.ResultCode = ELandscapeImportResult::Error;
        } else {
            if((strncmp(GDALGetDataTypeName(rasterBands[0]->GetRasterDataType()), "Int16", 6) != 0) &&
				(strncmp(GDALGetDataTypeName(rasterBands[0]->GetRasterDataType()), "Float32", 8) != 0)) {
                // neither 16 bit int nor 32 bit float
                Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_BitDepthError","Pixelformat is not 'Int16' or 'Float32'!");
                Result.ResultCode = ELandscapeImportResult::Error;
            } else {
                auto srs = gdaldata->GetProjectionRef();
                if(!FGeoReference::IsWGS84(OSRNewSpatialReference(srs)) &&
                   !FGeoReference::IsUTM(OSRNewSpatialReference(srs))) {
                    // not in wgs 84 or utm coordinate system
                    Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_SpatialReferenceError","Spatial Reference not supported!");
                    Result.ResultCode = ELandscapeImportResult::Error;
                } else {
                    double width, height;
                    FGeoReference::GetSize(gdaldata, width, height);
                    FLandscapeFileResolution ImportResolution;
                    ImportResolution.Width = width;
                    ImportResolution.Height = height;
                    Result.PossibleResolutions.Add(ImportResolution);
                    Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_ValidateSuccess","File validated");
                    Result.ResultCode = ELandscapeImportResult::Success;
                }
            }
        }
    }
    return Result;
}

FLandscapeHeightmapImportData FLandscapeHeightmapFileFormat_Geotiff::Import(const TCHAR* HeightmapFilename, FLandscapeFileResolution ExpectedResolution) const
{
    FLandscapeHeightmapImportData Result;

    TArray<uint8> TempData;

    GDALDatasetRef gdaldata = GDALHelpers::OpenRaster(HeightmapFilename, true);

    if(!gdaldata){
        // file open error
    } else {
        // get the first raster band
        GDALRasterBandH rasterBand = gdaldata->GetRasterBand(1);

        auto rasterBands = mergetiff::DatasetManagement::getRasterBands(gdaldata, {1});

        // Check if grayscale
        if(strncmp(GDALGetColorInterpretationName(rasterBands[0]->GetColorInterpretation()), "Gray", 5) != 0) {
            // pixel type error

        } else {
            // Check pixel format
            if(strncmp(GDALGetDataTypeName(rasterBands[0]->GetRasterDataType()), "Int16", 6) == 0) {
                double heightscale;
                UGeotiffRasterScaler::ConvertScaleGeotiffRaster<int16_t>(EPixelScanMode::G16_RG8, gdaldata, 1, TempData, ExpectedResolution.Width, ExpectedResolution.Height, heightscale);

                if(heightscale>1) {
                    Result.ResultCode = ELandscapeImportResult::Warning;
                    Result.ErrorMessage = FText::Format(LOCTEXT("FileFormatGeotiff_ImportLargeHeight", "The heightmap file appears to represent more than 655.33m in height. Suggested Z-Scale for Landscape to achieve realistic heights is {0}"), heightscale*100);
                }

                Result.Data.Empty(ExpectedResolution.Width * ExpectedResolution.Height);
                Result.Data.AddUninitialized(ExpectedResolution.Width * ExpectedResolution.Height);
                FMemory::Memcpy(Result.Data.GetData(), TempData.GetData(), ExpectedResolution.Width * ExpectedResolution.Height * 2);
			}
			else if (strncmp(GDALGetDataTypeName(rasterBands[0]->GetRasterDataType()), "Float32", 8) == 0) {
				double heightscale;
				UGeotiffRasterScaler::ConvertScaleGeotiffRaster<float>(EPixelScanMode::F32_RG8, gdaldata, 1, TempData, ExpectedResolution.Width, ExpectedResolution.Height, heightscale);

				if (heightscale > 1) {
					Result.ResultCode = ELandscapeImportResult::Warning;
					Result.ErrorMessage = FText::Format(LOCTEXT("FileFormatGeotiff_ImportLargeHeight", "The heightmap file appears to represent more than 655.33m in height. Suggested Z-Scale for Landscape to achieve realistic heights is {0}"), heightscale * 100);
				}

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
    GDALDatasetRef gdaldata = GDALHelpers::OpenRaster(WeightmapFilename, true);

    if(!gdaldata){
        // file open error
        Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_FileOpenError","Error opening file!");
        Result.ResultCode = ELandscapeImportResult::Error;
    } else {
        // get the first raster band
        GDALRasterBandH rasterBand = gdaldata->GetRasterBand(1);

        auto rasterBands = mergetiff::DatasetManagement::getRasterBands(gdaldata, {1});
        // Check if grayscale
        if (strncmp(GDALGetColorInterpretationName(GDALGetRasterColorInterpretation(rasterBand)), "Gray", 5) != 0) {
            // not a grayscale band
            Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_PixelTypeError","First Rasterband is not grayscale!");
            Result.ResultCode = ELandscapeImportResult::Error;
        } else {
            if(strncmp(GDALGetDataTypeName(GDALGetRasterDataType(rasterBand)), "Byte", 5) != 0) {
                // not 16 bit
                Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_BitDepthError","Pixelformat is not 'Byte'!");
                Result.ResultCode = ELandscapeImportResult::Error;
            } else {
                auto srs = gdaldata->GetProjectionRef();
                if(!FGeoReference::IsWGS84(OSRNewSpatialReference(srs)) &&
                   !FGeoReference::IsUTM(OSRNewSpatialReference(srs))){
                    Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_SpatialReferenceError","Spatial Reference not supported!");
                    Result.ResultCode = ELandscapeImportResult::Error;
                } else {
                    double width, height;
                    FGeoReference::GetSize(gdaldata, width, height);
                    FLandscapeFileResolution ImportResolution;
                    ImportResolution.Width = width;
                    ImportResolution.Height = height;
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

    TArray<uint8> TempData;

    // Open the file
    GDALDatasetRef gdaldata = GDALHelpers::OpenRaster(WeightmapFilename, true);
    if(!gdaldata){
        // file open error
    } else {
        // get the first raster band
        //GDALRasterBandH  rasterBand = gdaldata->GetRasterBand(1);
        auto rasterBands = mergetiff::DatasetManagement::getRasterBands(gdaldata, {1});

        // Check if grayscale
        if(strncmp(GDALGetColorInterpretationName(rasterBands[0]->GetColorInterpretation()), "Gray", 5) != 0) {
            // pixel type error

        } else {
            // Check pixel format
            if(strncmp(GDALGetDataTypeName(rasterBands[0]->GetRasterDataType()), "Byte", 5) != 0) {

            } else {
                double heightscale;
                UGeotiffRasterScaler::ConvertScaleGeotiffRaster<uint8_t>(EPixelScanMode::G8_G8, gdaldata, 1, TempData, ExpectedResolution.Width, ExpectedResolution.Height, heightscale);
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
