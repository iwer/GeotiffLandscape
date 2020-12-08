// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "S2GLCWeightmapAssetFactoryNew.generated.h"

UCLASS(hidecategories=Object)
class US2GLCWeightmapAssetFactoryNew
    : public UFactory
{
    GENERATED_BODY()
public:
    US2GLCWeightmapAssetFactoryNew(const FObjectInitializer& ObjectInitializer);
    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
};
