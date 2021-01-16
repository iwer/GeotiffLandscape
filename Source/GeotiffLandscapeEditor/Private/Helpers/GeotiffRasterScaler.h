// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once

#include "GDALHelpers.h"
#include "S2GLCClasses.h"
#include "UObject/Class.h"
#include "GeotiffRasterScaler.generated.h"

UENUM()
enum EPixelScanMode {
    G8_ValueMask8,  // Mask for single value in 8bit grayscale to 8bit bw (if pixel in source is MaskValue, pixel in target will be white. black if not)
    G8_BGRA8,       // 8bit grayscale to 8bit BGRA (B=G=R, A=1)
    G8_G8,          // 8bit grayscale as in source (without interpolation)
    G16_RG8,        // 16bit grayscale to two 8bits
    G16_BGRA8,      // 16bit grayscale to 8bit BGRA (B=0 G=LSB R=MSB A=1)
    F32_RG8,		// 32bit float to two 8bits
    F32_BGRA8		// 32bit float to  8bit BGRA (B=0 G=LSB R=MSB A=1)
};

UCLASS()
class UGeotiffRasterScaler
    : public UObject
{
    GENERATED_BODY()
public:
    constexpr static double UE4LANDSCAPE_HEIGHTLIMIT = 655.33;
    template  <typename PixelType>
    static void ConvertScaleGeotiffRaster(EPixelScanMode psm,
                                          GDALDatasetRef &source,
                                          int bandIndex,
                                          TArray<uint8> &targetPixels,
                                          int targetWidth,
                                          int targetHeight,
                                          double &ScaleSuggestion)
    {
        targetPixels.Empty();
        TargetToSourceScaling<PixelType>(psm, source, bandIndex, -1, targetPixels, targetWidth, targetHeight, ScaleSuggestion);
    }

    static bool ImportScaleGeotiffWeightLayer(GDALDatasetRef &source,
                                              int bandIndex,
                                              FName LayerName,
                                              TArray<uint8> &targetPixels,
                                              int targetWidth,
                                              int targetHeight)
    {
        const UEnum* classenum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ES2GLCClasses"), true);
        if (classenum) {
            int32 Index = classenum->GetIndexByName(LayerName);
            if(Index != INDEX_NONE) {
                uint8 layer_tifcolor = FS2GLCColors::TifCol[Index];
                UE_LOG(LogTemp,Warning,TEXT("Tif value for layer %s is %d"), *LayerName.ToString(), layer_tifcolor);
                // go on scanning
                targetPixels.Empty();

                double ScaleSuggestion;
                TargetToSourceScaling<uint8_t>(EPixelScanMode::G8_ValueMask8, source, bandIndex, layer_tifcolor, targetPixels, targetWidth, targetHeight, ScaleSuggestion);
            } else {
                UE_LOG(LogTemp,Warning,TEXT("Unknown Layername: %s"), *LayerName.ToString());
                return false;
            }
        } else {
            UE_LOG(LogTemp,Warning,TEXT("Object 'ES2GLCClasses' not found!"));
            return false;
        }
        return true;
    }

    static double HeightScaleFactor(double maxHeight, double minHeight)
    {
        // check if height difference is bigger than UE4 landscape height limit
        double heightdiff = maxHeight - minHeight;
        if(heightdiff > UE4LANDSCAPE_HEIGHTLIMIT) {
            return heightdiff / UE4LANDSCAPE_HEIGHTLIMIT;
        } else {
            return 1.0;
        }
    }

private:
    static float BilinearInterpolation(float A,
                                       float B,
                                       float C,
                                       float D,
                                       float xdiff,
                                       float ydiff)
    {
        return (A * (1 - xdiff) * (1 - ydiff) +  B * (xdiff) * (1 - ydiff) +
                C * (ydiff)     * (1 - xdiff) +  D * (xdiff  * ydiff));
    }

    template  <typename PixelType>
    static void TargetToSourceScaling(EPixelScanMode psm,
                                      GDALDatasetRef &source,
                                      int bandIndex,
                                      int16 S2GLCLayer,
                                      TArray<uint8> &targetPixels,
                                      int targetWidth,
                                      int targetHeight,
                                      double &ScaleSuggestion)
    {
        // determine min and max height
        RasterMinMaxRef rasterMinMax = GDALHelpers::ComputeRasterMinMax(source, bandIndex);

        ScaleSuggestion = HeightScaleFactor(rasterMinMax->Max, rasterMinMax->Min);

        UE_LOG(LogTemp,Warning,TEXT("ScaleSuggestion: %f, min: %f, max: %f"), ScaleSuggestion, rasterMinMax->Min, rasterMinMax->Max);

        auto rasterdata = mergetiff::DatasetManagement::rasterFromDataset<PixelType>(source,{static_cast<unsigned int>(bandIndex)});

        float xRatio = ((float)rasterdata.cols()-1)/((float)targetWidth);
        float yRatio = ((float)rasterdata.rows()-1)/((float)targetHeight);

        UE_LOG(LogTemp, Warning, TEXT("Rasterdata has %d channels, %dx%d -> %fx%f"), rasterdata.channels(),rasterdata.cols(), rasterdata.rows(), xRatio, yRatio);

        for(size_t targetY = 0; targetY < targetHeight; targetY++){
            // calculate source y
            int source_y = (int)(yRatio * targetY);
            // partial y-pixel for interpolation
            float y_diff = (yRatio * targetY) - source_y;
            for(size_t targetX = 0; targetX < targetWidth; targetX++){
                //calculate source x
                int source_x = (int)(xRatio * targetX);
                // partial x pixel for interpolation
                float x_diff = (xRatio * targetX) - source_x;

                // pixel index in continous array
                int pix_idx = source_y * rasterdata.cols() + source_x;

                // get pixels
                //     | x | x+1 |
                //----------------
                // y   | A | B   |
                //----------------
                // y+1 | C | D   |
                //----------------
                float A = rasterdata.pixelComponent(source_y,   source_x  , 0);
                float B = rasterdata.pixelComponent(source_y,   source_x+1, 0);
                float C = rasterdata.pixelComponent(source_y+1, source_x  , 0);
                float D = rasterdata.pixelComponent(source_y+1, source_x+1, 0);


                // when scanning for a selective value, put 8 bit into buffer
                // when scanning heightmaps use two bytes as 16 bit
                if(psm == EPixelScanMode::G8_ValueMask8 && S2GLCLayer != -1){
                    // color pixel white when it has Layer color, black otherwise
                    A = LayerOnPixel(S2GLCLayer, A);
                    B = LayerOnPixel(S2GLCLayer, B);
                    C = LayerOnPixel(S2GLCLayer, C);
                    D = LayerOnPixel(S2GLCLayer, D);
                    uint8 igray = BilinearInterpolation(A,B,C,D,x_diff,y_diff);
                    targetPixels.Add(igray);
                } else if (psm == EPixelScanMode::G8_G8){
                    //float gray = BilinearInterpolation(A,B,C,D,x_diff,y_diff);
                    uint8 igray = A;
                    targetPixels.Add(igray);
                } else if (psm == EPixelScanMode::G16_RG8 || psm == EPixelScanMode::F32_RG8){
                    float gray = BilinearInterpolation(A,B,C,D,x_diff,y_diff);
                    // scale to min height and max height
                    uint16 igray = (gray - rasterMinMax->Min) * 100 / HeightScaleFactor(rasterMinMax->Max, rasterMinMax->Min);

                    // split to two 8bits
                    uint8 gright = igray & 0xff;
                    uint8 gleft = (igray >> 8);

                    targetPixels.Add(gright);
                    targetPixels.Add(gleft);
                } else if (psm == EPixelScanMode::G8_BGRA8) {
                    float gray = BilinearInterpolation(A,B,C,D,x_diff,y_diff);
                    uint8 igray = gray;
                    targetPixels.Add(igray);
                    targetPixels.Add(igray);
                    targetPixels.Add(igray);
                    targetPixels.Add(igray);
                } else if (psm == EPixelScanMode::G16_BGRA8) {
                    float gray = BilinearInterpolation(A,B,C,D,x_diff,y_diff);
                    // scale to min height and max height
                    //uint16 igray = (gray - rasterMinMax->Min) * 100 / HeightScaleFactor(rasterMinMax->Max, rasterMinMax->Min);
                    uint16 igray = (gray - rasterMinMax->Min)  / (rasterMinMax->Max - rasterMinMax->Min) * 65535;

                    // split to two 8bits
                    uint8 gright = igray & 0xff;
                    uint8 gleft = (igray >> 8);

                    targetPixels.Add(0);
                    targetPixels.Add(gright);
                    targetPixels.Add(gleft);
                    targetPixels.Add(255);
                } else if (psm == EPixelScanMode::F32_BGRA8) {
                    float gray = BilinearInterpolation(A, B, C, D, x_diff, y_diff);
                    // scale to min height and max height
                    uint16 igray = (gray - rasterMinMax->Min) / (rasterMinMax->Max - rasterMinMax->Min) * 65535;

                    // split to two 8bits
                    uint8 gright = igray & 0xff;
                    uint8 gleft = (igray >> 8);

                    targetPixels.Add(0);
                    targetPixels.Add(gright);
                    targetPixels.Add(gleft);
                    targetPixels.Add(255);
                }
            }
        }
    }

    static uint8 LayerOnPixel(int16 S2GLCLayer, float Pixel)
    {
        if(uint8(Pixel) == S2GLCLayer) {
            return 255;
        } else if(S2GLCLayer == 123 && uint8(Pixel) == 0) { // interpret clouds as snow for now
            return 255;
        } else {
            return 0;
        }
    }
};
