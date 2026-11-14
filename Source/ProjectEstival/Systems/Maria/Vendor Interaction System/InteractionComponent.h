// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionEvent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable)
	bool StartInteract();
	
	UFUNCTION(BlueprintCallable)
	void StopInteract();
	
	UFUNCTION(BlueprintPure)
	bool IsHolding() const { return HoldTarget.IsValid(); }
	
	UFUNCTION(BlueprintPure)
	float GetHoldProgress() const;
	
	UPROPERTY(BlueprintAssignable) FOnInteractionEvent OnHoldStarted;
	UPROPERTY(BlueprintAssignable) FOnInteractionEvent OnHoldCancelled;
	UPROPERTY(BlueprintAssignable) FOnInteractionEvent OnInteracted;

protected:
	UPROPERTY(EditAnywhere, Category = "Interaction", meta = (ClampMin = "o"))
	float InteractRange = 200.f;
	
	UPROPERTY(EditAnywhere, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> InteractChannel = ECC_Pawn;

private:
	TWeakObjectPtr<UObject> HoldTarget;
	float HoldElapsed = 0.f;
	float HoldDuration = 0.f;
	
	UObject* FindNearestInteractable() const;
	AActor* GetTargetActor(const UObject* Target) const;
	bool IsHoldValid() const;
	void ResetHold();
};
