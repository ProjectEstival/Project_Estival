// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ControllerBase.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

UCLASS(Abstract)
class PROJECTESTIVAL_API AControllerBase : public APlayerController {
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = Input)
	TObjectPtr<UInputMappingContext> DefaultMappingContexts;
	
	//setup
	virtual void SetupInputComponent() override;
};

