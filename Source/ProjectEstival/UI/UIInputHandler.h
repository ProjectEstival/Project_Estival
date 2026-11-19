#pragma once

#include "UObject/Interface.h"
#include "CoreMinimal.h"

#include "UIInputHandler.generated.h"

class APlayerController;

UENUM(BlueprintType)
enum class EUIInputAction : uint8
{
	Submit,
	Back,
	PreviousAbility,
	NextAbility,
	PreviousCharacter,
	NextCharacter
};

UINTERFACE(BlueprintType)
class PROJECTESTIVAL_API UUIInputHandler : public UInterface
{
	GENERATED_BODY()
	
};

class PROJECTESTIVAL_API IUIInputHandler
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UI|Input")
	bool HandleUIInputAction(EUIInputAction Action, APlayerController* Instigator);
};
