#include "Engine/Texture2D.h"
#include "gdal_priv.h"
#include "cpl_conv.h"
#include "ogr_spatialref.h"
#include "ogr_geometry.h"

class GeotiffFileHelper {
public:
    static bool LoadFile(const FString &filename, UTexture2D * texture, FGeoRegion &region, float &minHeight, float &maxHeight)
    {
        GDALDatasetH gdaldata;
        gdaldata = GDALOpen(TCHAR_TO_ANSI(*filename), GA_ReadOnly);
        if(gdaldata == nullptr) {
            UE_LOG(LogTemp, Warning, TEXT("GeotiffFileHelper: Error opening GeoTiff file."));
            return false;
        } else {
            UE_LOG(LogTemp, Warning, TEXT("GeotiffFileHelper: Opened GeoTiff file %s."), *filename);
        }
        GDALRasterBandH poBand;
        poBand = GDALGetRasterBand(gdaldata, 1 );


        // get source pixel size
        int width, height;
        GDALGetBlockSize(poBand, &width, &height);

        // determine geo location
        const char * projref = GDALGetProjectionRef(gdaldata);
        FString s_projref(projref);
        UE_LOG(LogTemp, Warning, TEXT( "    Projection is `%s'\n"), *s_projref );

        double adfGeoTransform[6];
        if( GDALGetGeoTransform(gdaldata, adfGeoTransform) == CE_None )
        {
            UE_LOG(LogTemp, Warning, TEXT( "    Origin = (%.6f,%.6f)\n"),
                   adfGeoTransform[0], adfGeoTransform[3] );
            UE_LOG(LogTemp, Warning, TEXT( "    Pixel Size = (%.6f,%.6f)\n"),
                   adfGeoTransform[1], adfGeoTransform[5] );
        }
        region.North = adfGeoTransform[3];
        region.West  = adfGeoTransform[0];
        region.South = adfGeoTransform[3] + width * adfGeoTransform[4] + height * adfGeoTransform[5];
        region.East  = adfGeoTransform[0] + width * adfGeoTransform[1] + height * adfGeoTransform[2];


        // get min and max height
        int             bGotMin, bGotMax;
        double          adfMinMax[2];
        adfMinMax[0] = GDALGetRasterMinimum( poBand, &bGotMin );
        adfMinMax[1] = GDALGetRasterMaximum( poBand, &bGotMax );
        if( ! (bGotMin && bGotMax) )
            GDALComputeRasterMinMax(poBand, FALSE, adfMinMax);

        minHeight = adfMinMax[0];
        maxHeight = adfMinMax[1];

        // get texture
        texture->PlatformData = new FTexturePlatformData();
        texture->PlatformData->SizeX = width;
        texture->PlatformData->SizeY = height;
        texture->PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;

        uint8* Pixels = new uint8[width * height * 4];

        // fill pixels
        float *pafScanline, *pafNextScanline;
        pafScanline = (float *) CPLMalloc(sizeof(float)*width);
        pafNextScanline = (float *) CPLMalloc(sizeof(float)*width);
        for (int32 i = 0; i < height; i++)
        {
            // scan current and next line in geotiff
            CPLErr err1 = GDALRasterIO( poBand, GF_Read, 0, i, width, 1,
                                        pafScanline, width, 1, GDT_Float32,
                                        0, 0 );
            CPLErr err2 = GDALRasterIO( poBand, GF_Read, 0, i+1, width, 1,
                                        pafNextScanline, width, 1, GDT_Float32,
                                        0, 0 );
            if(err1!=CPLE_None || err2!=CPLE_None){
                UE_LOG(LogTemp, Warning, TEXT("GeotiffFileHelper: Error scanning lines of first raster band of file %s."), *filename);
                return false;
            }

            for (int32 j = 0; j < width; j++)
            {
                float gray = pafScanline[j];

                // scale to min height and max height
                uint16 igray = (gray-minHeight)/(maxHeight-minHeight) * float(TNumericLimits<uint16>::Max());

                //Get the current pixel index
                int32 CurrentPixelIndex = ((i * width) + j);

                // write 16-bit depth value to red and green 8bit channel
                Pixels[4 * CurrentPixelIndex] = 0; //b
                Pixels[4 * CurrentPixelIndex + 1] = (uint8)(igray & 0xff); //g << LSB
                Pixels[4 * CurrentPixelIndex + 2] = (uint8)(igray >> 8);   //r << MSB
                Pixels[4 * CurrentPixelIndex + 3] = 0; //a
            }
        }




        // cleanup
        CPLFree(pafScanline);
        CPLFree(pafNextScanline);



        FTexture2DMipMap* Mip = new FTexture2DMipMap();
        texture->PlatformData->Mips.Add(Mip);
        Mip->SizeX = width;
        Mip->SizeY = height;
        Mip->BulkData.Lock(LOCK_READ_WRITE);
        uint8* TextureData = (uint8*)Mip->BulkData.Realloc(width * height * sizeof(uint8) * 4);
        FMemory::Memcpy(TextureData, Pixels, sizeof(uint8) * width * height * 4);
        Mip->BulkData.Unlock();

        return true;
    }
};
