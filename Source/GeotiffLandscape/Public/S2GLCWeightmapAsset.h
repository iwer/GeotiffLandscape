// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Texture2D.h"
#include "Engine/DataAsset.h"
#include "ROI.h"

#include "S2GLCWeightmapAsset.generated.h"
/**
 *
 */
UCLASS(BlueprintType, hidecategories=(Object))
class GEOTIFFLANDSCAPE_API US2GLCWeightmapAsset
    : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="S2GLCWeightmapAsset")
    TArray<UTexture2D*> LayerMaps;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "S2GLCWeightmapAsset")
	URegionOfInterest * ROI;
};
