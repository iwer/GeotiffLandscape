// Copyright (c) Iwer Petersen. All rights reserved.

#pragma once

#include "Engine.h"

#include "GeoRegion.generated.h"

USTRUCT(BlueprintType)
struct FGeoRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Longitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Latitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SizeM;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly)
    float West;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly)
    float East;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly)
    float South;

    UPROPERTY(VisibleAnywhere, BlueprintReadonly)
    float North;
};
