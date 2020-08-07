#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "GeotiffAssetFactoryNew.generated.h"

UCLASS(hidecategories=Object)
class UGeotiffAssetFactoryNew
    : public UFactory
{
    GENERATED_BODY()
public:
    UGeotiffAssetFactoryNew(const FObjectInitializer& ObjectInitializer);
    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool ShouldShowInNewMenu() const override;
};
