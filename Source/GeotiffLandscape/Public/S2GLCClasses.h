// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once

#include "Engine.h"
#include "S2GLCClasses.generated.h"


// This are the classes encoded in S2GLC geotiff files.
UENUM(BlueprintType)
enum ES2GLCClasses {
                      // tifVal   R,  G,  B,  A
    Clouds,           //  0       255,255,255,255,
    Artificial,       //  62      210,0,  0,  255,
    Cultivated,       //  73      253,211,39, 255,
    Vineyards,        //  75      176,91, 16, 255,
    Broadleaf,        //  82      35, 152,0,  255,
    Coniferous,       //  83      8,  98, 0,  255,
    Herbaceous,       //  102     249,150,39, 255,
    MoorsHeath,       //  103     141,139,0,  255,
    Sclerophyllous,   //  104     95, 53, 6,  255,
    Marsh,            //  105     149,107,196,255,
    Peatbog,          //  106     77, 37, 106,255,
    Natural,          //  121     154,154,154,255,
    Snow,             //  123     106,255,255,255,
    Water,            //  162     20, 69, 249,255,
    NoData            //  255     255,255,255,255,
};

USTRUCT(BlueprintType)
struct FS2GLCColors {
	GENERATED_BODY()
		static constexpr int ClassCount = 15;
		static constexpr int TifCol[] = {0,62,73,75,82,83,102,103,104,105,106,121,123,162,255};
};
