// Copyright (c) Iwer Petersen. All rights reserved.


#include "LandscapeFileFormatGeoTiff.h"
#include "HAL/UnrealMemory.h"
#include "GeotiffRasterScaler.h"
#include "S2GLCClasses.h"
#include "GeoReferenceHelper.h"

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
                if(!FGeoReferenceHelper::IsWGS84(OSRNewSpatialReference(srs)) &&
                   !FGeoReferenceHelper::IsUTM(OSRNewSpatialReference(srs))) {
                    // not in wgs 84 or utm coordinate system
                    Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_SpatialReferenceError","Spatial Reference not supported!");
                    Result.ResultCode = ELandscapeImportResult::Error;
                } else {
                    double width, height;
                    URegionOfInterest::GetSize(gdaldata, width, height);
                    FLandscapeFileResolution ImportResolution;
                    ImportResolution.Width = width;
                    ImportResolution.Height = height;

                    auto HeightMinMax = GDALHelpers::ComputeRasterMinMax(gdaldata, 1);
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

        double width, height;
        URegionOfInterest::GetSize(gdaldata, width, height);

        // Check if grayscale
        if(strncmp(GDALGetColorInterpretationName(rasterBands[0]->GetColorInterpretation()), "Gray", 5) != 0) {
            // pixel type error

        } else {
            // Check pixel format
            if(strncmp(GDALGetDataTypeName(rasterBands[0]->GetRasterDataType()), "Int16", 6) == 0) {
                double heightscale;
                UGeotiffRasterScaler::ConvertScaleGeotiffRaster<int16_t>(EPixelScanMode::G16_RG8, gdaldata, 1, TempData, ExpectedResolution.Width, ExpectedResolution.Height, heightscale);


                // auto PredMapSize = PredictLandscapeSize(ExpectedResolution.Width, ExpectedResolution.Height);
                // Result.ResultCode = ELandscapeImportResult::Warning;
                // Result.ErrorMessage = FText::Format(LOCTEXT("FileFormatGeotiff_ImportLargeHeight", "The needs to be scaled for correct sizing: X:{0} Y:{1} Z:{2}"),
                //                         ExpectedResolution.Width / PredMapSize.X * 100,
                //                         ExpectedResolution.Height / PredMapSize.Y * 100,
                //                         heightscale*100);


                Result.Data.Empty(ExpectedResolution.Width * ExpectedResolution.Height);
                Result.Data.AddUninitialized(ExpectedResolution.Width * ExpectedResolution.Height);
                FMemory::Memcpy(Result.Data.GetData(), TempData.GetData(), ExpectedResolution.Width * ExpectedResolution.Height * 2);
			}
			else if (strncmp(GDALGetDataTypeName(rasterBands[0]->GetRasterDataType()), "Float32", 8) == 0) {
				double heightscale;
				UGeotiffRasterScaler::ConvertScaleGeotiffRaster<float>(EPixelScanMode::F32_RG8, gdaldata, 1, TempData, ExpectedResolution.Width, ExpectedResolution.Height, heightscale);


                // auto PredMapSize = PredictLandscapeSize(ExpectedResolution.Width, ExpectedResolution.Height);
                // Result.ResultCode = ELandscapeImportResult::Warning;
                // Result.ErrorMessage = FText::Format(LOCTEXT("FileFormatGeotiff_ImportLargeHeight", "The needs to be scaled for correct sizing: X:{0} Y:{1} Z:{2}"),
                //                         ExpectedResolution.Width / PredMapSize.X * 100,
                //                         ExpectedResolution.Height / PredMapSize.Y * 100,
                //                         heightscale*100);

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

/**
* From Editor/LandscapeEditor/Private/NewLandscapeUtils.cpp
*/
FVector2D FLandscapeHeightmapFileFormat_Geotiff::PredictLandscapeSize(int width, int height) const
{
    int32 SectionSizes[6] = { 7, 15, 31, 63, 127, 255 };
    int32 NumSections[2] = { 1, 2 };

    int32 Width = width;
    int32 Height = height;

    int32 QuadsPerSection = 0;
    int32 SectionsPerComponent = 0;
    int32 ComponentCountX = 0;
    int32 ComponentCountY = 0;

    bool bFoundMatch = false;
    if(Width > 0 && Height > 0)
    {
        // Try to find a section size and number of sections that exactly matches the dimensions of the heightfield
        for(int32 SectionSizesIdx = UE_ARRAY_COUNT(SectionSizes) - 1; SectionSizesIdx >= 0; SectionSizesIdx--)
        {
            for(int32 NumSectionsIdx = UE_ARRAY_COUNT(NumSections) - 1; NumSectionsIdx >= 0; NumSectionsIdx--)
            {
                int32 ss = SectionSizes[SectionSizesIdx];
                int32 ns = NumSections[NumSectionsIdx];

                if(((Width - 1) % (ss * ns)) == 0 && ((Width - 1) / (ss * ns)) <= 32 &&
                        ((Height - 1) % (ss * ns)) == 0 && ((Height - 1) / (ss * ns)) <= 32)
                {
                    bFoundMatch = true;
                    QuadsPerSection = ss;
                    SectionsPerComponent = ns;
                    ComponentCountX = (Width - 1) / (ss * ns);
                    ComponentCountY = (Height - 1) / (ss * ns);

                    break;
                }
            }
            if(bFoundMatch)
            {
                break;
            }
        }

        if(!bFoundMatch)
        {
            // if there was no exact match, try increasing the section size until we encompass the whole heightmap
            const int32 CurrentSectionSize = QuadsPerSection;
            const int32 CurrentNumSections = SectionsPerComponent;
            for(int32 SectionSizesIdx = 0; SectionSizesIdx < UE_ARRAY_COUNT(SectionSizes); SectionSizesIdx++)
            {
                if(SectionSizes[SectionSizesIdx] < CurrentSectionSize)
                {
                    continue;
                }

                const int32 ComponentsX = FMath::DivideAndRoundUp((Width - 1), SectionSizes[SectionSizesIdx] * CurrentNumSections);
                const int32 ComponentsY = FMath::DivideAndRoundUp((Height - 1), SectionSizes[SectionSizesIdx] * CurrentNumSections);
                if(ComponentsX <= 32 && ComponentsY <= 32)
                {
                    bFoundMatch = true;
                    QuadsPerSection = SectionSizes[SectionSizesIdx];
                    //UISettings->NewLandscape_SectionsPerComponent = ;
                    ComponentCountX = ComponentsX;
                    ComponentCountY = ComponentsY;

                    break;
                }
            }
        }

        if(!bFoundMatch)
        {
            // if the heightmap is very large, fall back to using the largest values we support
            const int32 MaxSectionSize = SectionSizes[UE_ARRAY_COUNT(SectionSizes) - 1];
            const int32 MaxNumSubSections = NumSections[UE_ARRAY_COUNT(NumSections) - 1];
            const int32 ComponentsX = FMath::DivideAndRoundUp((Width - 1), MaxSectionSize * MaxNumSubSections);
            const int32 ComponentsY = FMath::DivideAndRoundUp((Height - 1), MaxSectionSize * MaxNumSubSections);

            bFoundMatch = true;
            QuadsPerSection = MaxSectionSize;
            SectionsPerComponent = MaxNumSubSections;
            ComponentCountX = ComponentsX;
            ComponentCountY = ComponentsY;

        }
    }
    return FVector2D(QuadsPerSection*SectionsPerComponent*ComponentCountX, QuadsPerSection*SectionsPerComponent*ComponentCountY);

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
                if(!FGeoReferenceHelper::IsWGS84(OSRNewSpatialReference(srs)) &&
                   !FGeoReferenceHelper::IsUTM(OSRNewSpatialReference(srs))){
                    Result.ErrorMessage = LOCTEXT("FileFormatGeotiff_SpatialReferenceError","Spatial Reference not supported!");
                    Result.ResultCode = ELandscapeImportResult::Error;
                } else {
                    double width, height;
                    URegionOfInterest::GetSize(gdaldata, width, height);
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
