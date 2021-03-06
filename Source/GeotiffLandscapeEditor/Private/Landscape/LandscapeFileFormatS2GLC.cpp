// Copyright (c) Iwer Petersen. All rights reserved.


#include "LandscapeFileFormatS2GLC.h"
#include "HAL/UnrealMemory.h"
#include "GeotiffRasterScaler.h"
#include "GeotiffColorHelper.h"
#include "GeoReferenceHelper.h"
#include "S2GLCClasses.h"

#define LOCTEXT_NAMESPACE "LandscapeEditor.NewLandscape"


FLandscapeWeightmapFileFormat_S2GLC::FLandscapeWeightmapFileFormat_S2GLC()
{
        FileTypeInfo.Description = LOCTEXT("FileFormatS2GLC_WeightmapDesc", "S2GLC Layer .s2glc files");
        FileTypeInfo.Extensions.Add(".s2glc");
        FileTypeInfo.bSupportsExport = false;
}

FLandscapeWeightmapInfo FLandscapeWeightmapFileFormat_S2GLC::Validate(const TCHAR* WeightmapFilename, FName LayerName) const
{
    FLandscapeWeightmapInfo Result;

    //check if LayerName is a valid S2GLC class
    const UEnum* classenum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ES2GLCClasses"), true);
    if (classenum) {
        int32 Index = classenum->GetIndexByName(LayerName);
        if(Index == INDEX_NONE) {
            Result.ErrorMessage = LOCTEXT("FileFormatS2GLC_LayerNameError","LayerName is not a valid S2GLC class!");
            Result.ResultCode = ELandscapeImportResult::Error;
        } else {
            UE_LOG(LogTemp,Warning,TEXT("Importing Layer %s, S2GLCIndex %d"), *LayerName.ToString(), Index);
            // Open the file
            GDALDatasetRef gdaldata = GDALHelpers::OpenRaster(WeightmapFilename, true);

            if(!gdaldata){
                // file open error
                Result.ErrorMessage = LOCTEXT("FileFormatS2GLC_FileOpenError","Error opening file!");
                Result.ResultCode = ELandscapeImportResult::Error;
            } else {
                // get the first raster band
                GDALRasterBandH rasterBand = gdaldata->GetRasterBand(1);

                auto rasterBands = mergetiff::DatasetManagement::getRasterBands(gdaldata, {1});
                // Check if grayscale
                if (strncmp(GDALGetColorInterpretationName(rasterBands[0]->GetColorInterpretation()), "Gray", 5) != 0) {
                    // not a grayscale band
                    Result.ErrorMessage = LOCTEXT("FileFormatS2GLC_PixelTypeError","First Rasterband is not grayscale!");
                    Result.ResultCode = ELandscapeImportResult::Error;
                } else {
                    // TODO: check if color table matches S2GLC classes
                    if(!UGeotiffColorHelper::VerifyS2GLCColors(gdaldata)) {
                        Result.ErrorMessage = LOCTEXT("FileFormatS2GLC_ColorTableError","Color table does not match S2GLC classes!");
                        Result.ResultCode = ELandscapeImportResult::Error;
                    } else {
                        // Check raster data type
                        if(strncmp(GDALGetDataTypeName((rasterBands[0]->GetRasterDataType())), "Byte", 5) != 0) {
                            // not 16 bit
                            Result.ErrorMessage = LOCTEXT("FileFormatS2GLC_BitDepthError","Pixelformat is not 'Byte'!");
                            Result.ResultCode = ELandscapeImportResult::Error;
                        } else {
                            auto srs = gdaldata->GetProjectionRef();
                            if(!FGeoReferenceHelper::IsWGS84(OSRNewSpatialReference(srs)) &&
                               !FGeoReferenceHelper::IsUTM(OSRNewSpatialReference(srs))){
                                // not in wgs 84 coordinate system
                                Result.ErrorMessage = LOCTEXT("FileFormatS2GLC_SpatialReferenceError","Spatial Reference not supported!");
                                Result.ResultCode = ELandscapeImportResult::Error;
                            } else {
                                double width, height;
                                URegionOfInterest::GetSize(gdaldata, width, height);
                                FLandscapeFileResolution ImportResolution;
                                ImportResolution.Width = width;
                                ImportResolution.Height = height;
                                Result.PossibleResolutions.Add(ImportResolution);
                                Result.ErrorMessage = LOCTEXT("FileFormatS2GLC_ValidateSuccess","File validated");
                                Result.ResultCode = ELandscapeImportResult::Success;
                            }
                        }
                    }
                }
            }
        }
    } else {
        Result.ErrorMessage = LOCTEXT("FileFormatS2GLC_InternalError","Could not find S2GLCClasses object!");
        Result.ResultCode = ELandscapeImportResult::Error;
    }


    return Result;
}

FLandscapeWeightmapImportData FLandscapeWeightmapFileFormat_S2GLC::Import(const TCHAR* WeightmapFilename, FName LayerName, FLandscapeFileResolution ExpectedResolution) const
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
                if(!UGeotiffRasterScaler::ImportScaleGeotiffWeightLayer(gdaldata, 1, LayerName, TempData, ExpectedResolution.Width, ExpectedResolution.Height)){
                    Result.ResultCode = ELandscapeImportResult::Error;
                    Result.ErrorMessage = LOCTEXT("FileFormatS2GLC_ImportFailure", "Weightmap import failed!");
                } else {
                    Result.Data.Empty(ExpectedResolution.Width * ExpectedResolution.Height);
                    Result.Data.AddUninitialized(ExpectedResolution.Width * ExpectedResolution.Height);
                    FMemory::Memcpy(Result.Data.GetData(), TempData.GetData(), ExpectedResolution.Width * ExpectedResolution.Height);
                }
            }
        }
    }

    return Result;
}

void FLandscapeWeightmapFileFormat_S2GLC::Export(const TCHAR* WeightmapFilename, FName LayerName, TArrayView<const uint8> Data, FLandscapeFileResolution DataResolution) const
{

}

#undef LOCTEXT_NAMESPACE
