// Copyright (c) Iwer Petersen. All rights reserved.
#include "S2GLCWeightmapAssetFactoryNew.h"

#include "S2GLCWeightmapAsset.h"

US2GLCWeightmapAssetFactoryNew::US2GLCWeightmapAssetFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = US2GLCWeightmapAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
    Formats.Add(TEXT("s2glc;S2GLC Weightmap File"));
}

UObject* US2GLCWeightmapAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<US2GLCWeightmapAsset>(InParent, InClass, InName, Flags);
}


bool US2GLCWeightmapAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
