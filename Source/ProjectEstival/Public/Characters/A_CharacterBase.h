// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "A_CharacterBase.generated.h"

class UInputAction;
class UAC_DashComponent;
class UAC_AbilityComponent;
class UAC_LookAtComponent;
class UAC_StatusEffectComponent;

/**
 *  Base class for every Estival character
 */
UCLASS(Abstract)
class PROJECTESTIVAL_API AA_EstivalBaseCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UAC_DashComponent> Dash;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UAC_AbilityComponent> PrimaryAbility;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UAC_AbilityComponent> SecondaryAbility;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UAC_LookAtComponent> LookAt;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UAC_StatusEffectComponent> StatusEffects;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> PrimaryAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> SecondaryAction;

public:

	AA_EstivalBaseCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Moves relative to the control rotation. Usable by input, AI or Blueprint */
	UFUNCTION(BlueprintCallable, Category="Movement")
	void DoMove(float AxisX, float AxisY);
};