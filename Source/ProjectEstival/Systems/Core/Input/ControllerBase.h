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
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, Category = Input)
	TObjectPtr<UInputAction> MoveAction;
	
	//setup
	virtual void SetupInputComponent() override;
	
private:
	
	//reference to const value to avoid changing the original value with &
	void Move(const FInputActionValue& Value);
	
};

