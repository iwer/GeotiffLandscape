// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "GeotiffHeightmapAssetFactoryNew.generated.h"

UCLASS(hidecategories=Object)
class UGeotiffHeightmapAssetFactoryNew
    : public UFactory
{
    GENERATED_BODY()
public:
    UGeotiffHeightmapAssetFactoryNew(const FObjectInitializer& ObjectInitializer);
    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
    virtual bool ShouldShowInNewMenu() const override;
};
