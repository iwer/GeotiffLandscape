// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Texture2D.h"
#include "Engine/DataAsset.h"
#include "GeoRegion.h"

#include "GeotiffAsset.generated.h"
/**
 *
 */
UCLASS(BlueprintType, hidecategories=(Object))
class GEOTIFFHEIGHTMAP_API UGeotiffAsset
    : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GeotiffAsset")
    float MinHeight;
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GeotiffAsset")
    float MaxHeight;
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GeotiffAsset")
    UTexture2D * Texture;
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="GeotiffAsset")
    FGeoRegion GeoRegion;
};
