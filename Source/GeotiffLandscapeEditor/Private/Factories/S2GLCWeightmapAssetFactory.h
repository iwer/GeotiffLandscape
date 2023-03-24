// Copyright (c) Iwer Petersen. All rights reserved.
#pragma once

#include "SmartPointers.h"
#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "S2GLCWeightmapAssetFactory.generated.h"

UCLASS(BlueprintType, hidecategories=Object)
class US2GLCWeightmapAssetFactory
    : public UFactory
{
    GENERATED_BODY()
public:
    US2GLCWeightmapAssetFactory(const FObjectInitializer& ObjectInitializer);
    virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
    virtual bool FactoryCanImport(const FString & Filename) override;
private:
    void ExtractLayerMap(GDALDatasetRef &Source, UTexture2D * TargetTexture, FName LayerName, TArray<uint8> Pixels, int Width, int Height) const;
};
