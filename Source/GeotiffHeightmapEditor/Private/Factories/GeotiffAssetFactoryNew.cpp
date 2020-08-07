#include "GeotiffAssetFactoryNew.h"

#include "GeotiffAsset.h"

UGeotiffAssetFactoryNew::UGeotiffAssetFactoryNew(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UGeotiffAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UGeotiffAssetFactoryNew::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UGeotiffAsset>(InParent, InClass, InName, Flags);
}


bool UGeotiffAssetFactoryNew::ShouldShowInNewMenu() const
{
	return true;
}
