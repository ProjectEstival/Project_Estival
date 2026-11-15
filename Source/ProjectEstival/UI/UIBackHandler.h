#pragma once

#include "UObject/Interface.h"
#include "CoreMinimal.h"

#include "UIBackHandler.generated.h"

class APlayerController;

UINTERFACE(BlueprintType)
class PROJECTESTIVAL_API UUIBackHandler : public UInterface
{
	GENERATED_BODY()
};

// Implement this on modals if you want to override the universal 'back' logic
class PROJECTESTIVAL_API IUIBackHandler
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|Navigation")
	bool HandleBack(APlayerController* Instigator);
	
	// Called after the HUD performs its normal pop because HandleBack returned false
	UFUNCTION(BlueprintNativeEvent, Category = "UI|Navigation")
	void OnBackPopped(APlayerController* Instigator);
};
