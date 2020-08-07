#pragma once

#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "GeotiffAssetFactory.generated.h"

UCLASS(hidecategories=Object)
class UGeotiffAssetFactory
    : public UFactory
{
    GENERATED_BODY()
public:
    UGeotiffAssetFactory(const FObjectInitializer& ObjectInitializer);
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
};
