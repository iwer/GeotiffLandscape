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
    constexpr static double UE4LANDSCAPE_HEIGHT_LIMIT = 655.33;
    template  <typename PixelType>
    static void ConvertScaleGeotiffRaster(const EPixelScanMode Psm,
                                          GDALDatasetRef &Source,
                                          const int BandIndex,
                                          TArray<uint8> &TargetPixels,
                                          const int TargetWidth,
                                          const int TargetHeight,
                                          double &ScaleSuggestion)
    {
        TargetPixels.Empty();
        TargetToSourceScaling<PixelType>(Psm, Source, BandIndex, -1, TargetPixels, TargetWidth, TargetHeight, ScaleSuggestion);
    }

    static bool ImportScaleGeotiffWeightLayer(GDALDatasetRef &Source,
                                              const int BandIndex,
                                              const FName LayerName,
                                              TArray<uint8> &TargetPixels,
                                              const int TargetWidth,
                                              const int TargetHeight)
    {
        const UEnum* ClassEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ES2GLCClasses"), true);
        if (ClassEnum) {
            const int32 Index = ClassEnum->GetIndexByName(LayerName);
            if(Index != INDEX_NONE) {
                const uint8 Layer_TifColor = FS2GLCColors::TifCol[Index];
                UE_LOG(LogTemp,Warning,TEXT("Tif value for layer %s is %d"), *LayerName.ToString(), Layer_TifColor);
                // go on scanning
                TargetPixels.Empty();

                double ScaleSuggestion;
                TargetToSourceScaling<uint8_t>(EPixelScanMode::G8_ValueMask8, Source, BandIndex, Layer_TifColor, TargetPixels, TargetWidth, TargetHeight, ScaleSuggestion);
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

    static double HeightScaleFactor(const double MaxHeight, const double MinHeight)
    {
        // check if height difference is bigger than UE4 landscape height limit
        const double HeightDiff = MaxHeight - MinHeight;
        if(HeightDiff > UE4LANDSCAPE_HEIGHT_LIMIT) {
            return HeightDiff / UE4LANDSCAPE_HEIGHT_LIMIT;
        } else {
            return 1.0;
        }
    }

private:
    static float BilinearInterpolation(const float A,
                                       const float B,
                                       const float C,
                                       const float D,
                                       const float XDiff,
                                       const float YDiff)
    {
        return (A * (1 - XDiff) * (1 - YDiff) +  B * (XDiff) * (1 - YDiff) +
                C * (YDiff)     * (1 - XDiff) +  D * (XDiff  * YDiff));
    }

    template  <typename PixelType>
    static void TargetToSourceScaling(const EPixelScanMode Psm,
                                      GDALDatasetRef &Source,
                                      const int BandIndex,
                                      const int16 S2GlcLayer,
                                      TArray<uint8> &TargetPixels,
                                      const int TargetWidth,
                                      const int TargetHeight,
                                      double &ScaleSuggestion)
    {
        // determine min and max height
        const RasterMinMaxRef RasterMinMax = GDALHelpers::ComputeRasterMinMax(Source, BandIndex);

        ScaleSuggestion = HeightScaleFactor(RasterMinMax->Max, RasterMinMax->Min);

        UE_LOG(LogTemp,Warning,TEXT("ScaleSuggestion: %f, min: %f, max: %f"), ScaleSuggestion, RasterMinMax->Min, RasterMinMax->Max);

        auto RasterData = mergetiff::DatasetManagement::rasterFromDataset<PixelType>(Source,{static_cast<unsigned int>(BandIndex)});

        float XRatio = (static_cast<float>(RasterData.cols())-1)/static_cast<float>(TargetWidth);
        float YRatio = (static_cast<float>(RasterData.rows())-1)/static_cast<float>(TargetHeight);

        UE_LOG(LogTemp, Warning, TEXT("Rasterdata has %d channels, %dx%d -> %fx%f"), RasterData.channels(),RasterData.cols(), RasterData.rows(), XRatio, YRatio);

        for(size_t TargetY = 0; TargetY < TargetHeight; TargetY++){
            // calculate source y
            int Source_Y = static_cast<int>(YRatio * TargetY);
            // partial y-pixel for interpolation
            const float Y_Diff = (YRatio * TargetY) - Source_Y;
            for(size_t TargetX = 0; TargetX < TargetWidth; TargetX++){
                //calculate source x
                int Source_X = static_cast<int>(XRatio * TargetX);
                // partial x pixel for interpolation
                const float X_Diff = (XRatio * TargetX) - Source_X;

                // get pixels
                //     | x | x+1 |
                //----------------
                // y   | A | B   |
                //----------------
                // y+1 | C | D   |
                //----------------
                float A = RasterData.pixelComponent(Source_Y,   Source_X  , 0);
                float B = RasterData.pixelComponent(Source_Y,   Source_X+1, 0);
                float C = RasterData.pixelComponent(Source_Y+1, Source_X  , 0);
                float D = RasterData.pixelComponent(Source_Y+1, Source_X+1, 0);


                // when scanning for a selective value, put 8 bit into buffer
                // when scanning heightmaps use two bytes as 16 bit
                if(Psm == EPixelScanMode::G8_ValueMask8 && S2GlcLayer != -1){
                    // color pixel white when it has Layer color, black otherwise
                    A = LayerOnPixel(S2GlcLayer, A);
                    B = LayerOnPixel(S2GlcLayer, B);
                    C = LayerOnPixel(S2GlcLayer, C);
                    D = LayerOnPixel(S2GlcLayer, D);
                    uint8 IGray = BilinearInterpolation(A,B,C,D,X_Diff,Y_Diff);
                    TargetPixels.Add(IGray);
                } else if (Psm == EPixelScanMode::G8_G8){
                    //float gray = BilinearInterpolation(A,B,C,D,x_diff,y_diff);
                    uint8 IGray = A;
                    TargetPixels.Add(IGray);
                } else if (Psm == EPixelScanMode::G16_RG8 || Psm == EPixelScanMode::F32_RG8){
                    const float Gray = BilinearInterpolation(A,B,C,D,X_Diff,Y_Diff);
                    
                    // scale to min height and max height
                    uint16 IGray = (Gray + 327.67) * 100 / HeightScaleFactor(RasterMinMax->Max, RasterMinMax->Min);
                    
                    // split to two 8bits
                    uint8 GRight = IGray & 0xff;
                    uint8 GLeft = (IGray >> 8);

                    TargetPixels.Add(GRight);
                    TargetPixels.Add(GLeft);
                } else if (Psm == EPixelScanMode::G8_BGRA8) {
                    const float Gray = BilinearInterpolation(A,B,C,D,X_Diff,Y_Diff);
                    uint8 IGray = Gray;
                    TargetPixels.Add(IGray);
                    TargetPixels.Add(IGray);
                    TargetPixels.Add(IGray);
                    TargetPixels.Add(IGray);
                } else if (Psm == EPixelScanMode::G16_BGRA8) {
                    const float Gray = BilinearInterpolation(A,B,C,D,X_Diff,Y_Diff);
                    // scale to min height and max height
                    uint16 IGray = (Gray - RasterMinMax->Min)  / (RasterMinMax->Max - RasterMinMax->Min) * 65535;

                    // split to two 8bits
                    uint8 GRight = IGray & 0xff;
                    uint8 GLeft = (IGray >> 8);

                    TargetPixels.Add(0);
                    TargetPixels.Add(GRight);
                    TargetPixels.Add(GLeft);
                    TargetPixels.Add(255);
                } else if (Psm == EPixelScanMode::F32_BGRA8) {
                    const float Gray = BilinearInterpolation(A, B, C, D, X_Diff, Y_Diff);
                    // scale to min height and max height
                    uint16 IGray = (Gray - RasterMinMax->Min) / (RasterMinMax->Max - RasterMinMax->Min) * 65535;

                    // split to two 8bits
                    uint8 GRight = IGray & 0xff;
                    uint8 GLeft = (IGray >> 8);

                    TargetPixels.Add(0);
                    TargetPixels.Add(GRight);
                    TargetPixels.Add(GLeft);
                    TargetPixels.Add(255);
                }
            }
        }
    }

    static uint8 LayerOnPixel(int16 S2GLCLayer, float Pixel)
    {
        if(static_cast<uint8>(Pixel) == S2GLCLayer) {
            return 255;
        } else if(S2GLCLayer == 123 && static_cast<uint8>(Pixel) == 0) { // interpret clouds as snow for now
            return 255;
        } else {
            return 0;
        }
    }
};
