GeotiffLandscape - UE4 Plugin
==============================
Adds geotiff files as UE4 Landscape height/weight map file type and custom data asset type. This can be used to generate real scale UE4 maps from existing locations. Geotiff digital elevation models (DEM) contain information about geo coordinates of the map and information about maximum and minimum elevation. With this information 1:1 scale Landscapes can be created using this Plugin. Also geotiff land cover files from the [S2GLC](http://s2glc.cbk.waw.pl/) can be imported to named landscape layers.

## Features
* Landscape Heightmap Fileformat Geotiff
* Landscape Weightmap Fileformat Geotiff
* Landscape Weightmap Fileformat S2GLC
* Layered Landscape Material Example for S2GLC Layers
* GeotiffHeightmap Asset with 16bit of height in red and green channels of a Texture
* S2GLC Weightmap Asset with 12 gracscale layer maps

*WARNING: This is experimental research code that **will** be subject of change.*

This example was generated using a DEM from [Opentopography](https://www.opentopography.org/) and the corresponding section from S2GLC around this coordinates: 50.959057,14.061609.
![alt text](Resources/example0.png "Overview over the map.")
![alt text](Resources/example1.png "Example Rendering from close to ground.")
Landscape textures in the screenshots are from [CC0textures](https://cc0textures.com/) and [FreePBR](https://freepbr.com/). Foliage assets from [NaturePackage](https://unrealengine.com/marketplace/en-US/product/nature-package) (was free for April 2020 in UE4 Marketplace) and [PN_FoliageCollection](https://www.unrealengine.com/marketplace/en-US/product/interactive-foliage-collection).

## Cutbacks
DEMs are usually not available at high resolutions. The popular NASA SRTM GL1 data provides an average resolution of 30 meter per pixel while UE4 usually uses 1 pixel per meter for Landscapes. The low resolution geotiff texture is therefore scaled up using bi-linear interpolation to reflect their geo-spatial extends in UE4 scale (1 heightmappixel -> 1 meter of landscape). However artifacts of the low resolutions remain visible.

UE4 Landscapes usually support height differences of 655.33 meter. When the height difference of the geotiff is larger, the full range will be used to map the height values and a small yellow warning sign besides the filename suggest a Z-Scale to correct the scaling factor. This scale is best entered into the landscape z scale box before importing.

## Building

This plugin uses [GeoReference](https://github.com/iwer/GeoReference), [UnrealGDAL](https://github.com/TensorWorks/UnrealGDAL), [ue4cli](https://github.com/adamrehn/ue4cli) and [conan-ue4cli](https://github.com/adamrehn/conan-ue4cli) to build and manage the required GDAL libraries.

* Download a Release of UnrealGDAL and place it in the Plugins directory or clone and follow instructions to build UnrealGDAL from source.

## Usage

Add GeotiffLandscape to `.uproject` or `.uplugin`

```json
"Plugins": [
  {
    "Name": "GeotiffLandscape",
    "Enabled": true
  }
]
```

Add build dependencies to your modules

```csharp
PublicDependencyModuleNames.AddRange(
  new string[]
  {
    "GeotiffLandscape",
    "GeotiffLandscapeEditor"
  }
);
```

## Landscape import

### Landscape Heightmap import
In Landscape mode when importing a heightmap you will notice a new filetype "DEM Heightmap .tif files". Several checks are performed to ensure the file can be imported. Currently only geotiff with WGS84 or UTM geo reference containing a single Int16 rasterband layer of gray-scale values in meters are tested but other types may just work or require little adaptation of the import code.

### Landscape Weightmap import

TIF files can be interpreted in two ways for weightmap generation.

#### Single layer weightmap
The obvious interpretion of the grayscale values is as one weightmap. The grayscale values are mapped to a range of 0 to 1 that represent the layer weight.

#### Multi layer weightmap

[S2GLC](http://s2glc.cbk.waw.pl/) provides huge landcover geotiffs for europe. Using the geo reference of the heightmap we can crop the area of interest into a smaller file. This is currently done in [QGIS](https://www.qgis.org/) but may be integrated in the import process later. The gray scale values in this image represent 15 classes of landcover:
* Clouds
* Artificial
* Cultivated
* Vineyards
* Broadleaf
* Coniferous
* Herbaceous
* MoorsHeath
* Sclerophyllous
* Marsh
* Peatbog
* Natural
* Snow
* Water
* NoData

For every class a landscape layer can be created (The layers are automagically created when using the LMT_S2GLCExample material for the landscape (*except NoData and Clouds atm.*)). This material also shows how to implement landcover dependent foliage using the Landscape Grass Tool. When importing the weightmap into the layer, the name of the layer has to match one of the landcover classes to select the grayscale from the S2GLC geotiff.

A special implementation interpretes distinct grayscale values as distinct weightmaps. The resulting weightmaps have a weight of 1 where the TIF pixel has the matching grayscale value and 0 where it has not.

To enable UE4 to distinguish between regular TIF weightmaps and S2GLC weightmaps, this plugin introduces the file extension *.s2glc. This is basically a *.tif file that is supposed to only have the 15 S2GLC grayscale values.

## DataAssets

If you want to use render targets to generate the landscape at runtime you'll need all the height- and weightdata as textures and the geo reference metadata. For this purpose Geotiff heightmaps and S2GLC files can also be imported as DataAssets.

### Geotiff Heightmap DataAsset
Geotiff heightmap assets contain the geo information and a texture2D where the 16 bit height values are split into the red and green channel of the texture. Because .tif files can also be imported as Texture without the geo reference information it is important to select the file type "Geotiff File (.tif)" when importing an asset into the content browser otherwise the pure texture import will be auto-selected.

### S2GLC Weightmap DataAsset
S2GLC DataAssets contain a texture for each of the 15 layers (*except NoData and Clouds*) and the geo reference information.
