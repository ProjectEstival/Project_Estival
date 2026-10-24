// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "InputAction.h"
#include "Systems/Maria/ReviveComponent.h"
#include "PlayerBase.generated.h"

class UReviveComponent;
UCLASS()
class PROJECTESTIVAL_API APlayerBase : public AEstivalBaseCharacter
{
	GENERATED_BODY()

public:
	
	APlayerBase();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resurrection")
	TObjectPtr<UReviveComponent>ResurrectionComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Resurrection | Reviving")
	TObjectPtr<UInputAction>ReviveAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Resurrection | Reviving")
	float ReviveRange = 200.f;

private:
	void StartRevive();
	void StopRevive();
	UReviveComponent* FindReviveTarget() const;
	
	TWeakObjectPtr<UReviveComponent> ReviveTarget;
};
