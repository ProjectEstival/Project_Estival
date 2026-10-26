// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "InputAction.h"
#include "Systems/Maria/ReviveComponent.h"
#include "PlayerBase.generated.h"

UCLASS()
class PROJECTESTIVAL_API APlayerBase : public AEstivalBaseCharacter
{
	GENERATED_BODY()

public:
	
	APlayerBase();
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resurrection")
	TObjectPtr<UReviveComponent>ResurrectionComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Resurrection | Reviving")
	TObjectPtr<UInputAction>ReviveAction;

private:
	void StartRevive();
	void StopRevive();
};
