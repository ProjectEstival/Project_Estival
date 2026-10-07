// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "AC_ActionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionActivated);

/**
 *  Base for any activatable action
 *  Handles the shared cooldown and status blocking, children only implement Execute().
 */
UCLASS(Abstract, ClassGroup=(Estival))
class PROJECTESTIVAL_API UAC_ActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Time between two activations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Action", meta = (ClampMin = 0, Units = "s"))
	float Cooldown = 1.0f;

	/** Activation is refused while the owner has any of these status tags (e.g. Status.Debuff.Stunned) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Action")
	FGameplayTagContainer BlockedByTags;

	UPROPERTY(BlueprintAssignable, Category="Action")
	FOnActionActivated OnActivated;

	/** Activates the action if possible. Returns true on success */
	UFUNCTION(BlueprintCallable, Category="Action")
	bool TryActivate();

	UFUNCTION(BlueprintPure, Category="Action")
	virtual bool CanActivate() const;

	/** Seconds left before the action can be used again */
	UFUNCTION(BlueprintPure, Category="Action")
	float GetRemainingCooldown() const;

protected:

	/** Performs the action */
	virtual void Execute() PURE_VIRTUAL(UAC_ActionComponent::Execute, );

private:

	double LastActivationTime = -UE_BIG_NUMBER;
};