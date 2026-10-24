// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReviveComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResurrectionEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLivesChanged, int32, RemainingLives);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UReviveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UReviveComponent();
	
	UFUNCTION(BlueprintPure)
	bool CanBeRevived() const;
	
	UFUNCTION(BlueprintCallable)
	bool BeginResurrection(AActor* Reviver);
	
	UFUNCTION(BlueprintCallable)
	void CancelResurrection();
	
	UFUNCTION(BlueprintPure)
	bool IsInProgress() const {return bIsInProgress; }
	
	UFUNCTION(BlueprintPure)
	float GetProgress() const;
	
	UFUNCTION(BlueprintCallable)
	void ReviveAtLocation(const FVector& Location);
	
	UFUNCTION(BlueprintPure)
	int32 GetRemainingLives() const { return RemainingLives; }
	
	UPROPERTY(BlueprintAssignable) FOnResurrectionEvent OnStarted;
	UPROPERTY(BlueprintAssignable) FOnResurrectionEvent OnCancelled;
	UPROPERTY(BlueprintAssignable) FOnResurrectionEvent OnCompleted;
	UPROPERTY(BlueprintAssignable) FOnLivesChanged OnLivesChanged;

protected:
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(EditAnywhere, Category = "Resurrection", meta = (ClampMin = "0"))
	int32 MaxLives = 3;
	
	UPROPERTY(EditAnywhere, Category = "Resurrection", meta = (ClampMin = "0"))
	float Duration = 3.f;
	
	UPROPERTY(EditAnywhere, Category = "Resurrection", meta = (ClampMin = "0", ClampMax = "1"))
	float HealthPercent = .5f;
	
	UPROPERTY(EditAnywhere, Category = "Resurrection | Off-Screen", meta = (ClampMin = "0"))
	float OffScreenDelay = 1.5f;
	
	UPROPERTY(EditAnywhere, Category = "Resurrection | Off-Screen", meta = (ClampMin = "0.05"))
	float OffscreenCheckInterval = 0.25f;

private:
	
	UFUNCTION()
	void HandleOwnerDeath();
	
	UFUNCTION()
	void HandleOwnerRevived();
	
	void CheckOffScreen();
	bool IsOwnerOnScreen() const;
	bool FindPartyLocation(FVector& OutLocation) const;
	void Complete();
	
	int32 RemainingLives = 0;
	float OffscreenTime = 0.f;
	bool bIsInProgress = false;
	TWeakObjectPtr<AActor> CurrentReviver;
	FTimerHandle TimerHandle, OffscreenTimer;
};
