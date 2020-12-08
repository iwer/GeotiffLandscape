// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Texture2D.h"
#include "Engine/DataAsset.h"
#include "ROI.h"
#include "GeotiffHeightmapAsset.generated.h"
/**
 *
 */
UCLASS(BlueprintType, hidecategories=(Object))
class GEOTIFFLANDSCAPE_API UGeotiffHeightmapAsset
    : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GeotiffHeightmapAsset")
    float MinHeight;
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GeotiffHeightmapAsset")
    float MaxHeight;
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GeotiffHeightmapAsset")
    UTexture2D * Texture;
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GeotiffHeightmapAsset")
	URegionOfInterest * ROI;
};
