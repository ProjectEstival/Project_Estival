// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Systems/Maria/Revivable.h"
#include "PlayerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReviveCountChanged, int32, RemainingLives);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResurrectionEvent);

UCLASS()
class PROJECTESTIVAL_API APlayerBase : public AEstivalBaseCharacter, public IRevivable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerBase();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(BlueprintCallable, Category = "Resurrection")
	bool BeginResurrection();
	
	UFUNCTION(BlueprintCallable, Category = "Resurrection", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	bool BeginResurrectionWithSettings(float DurationSeconds, float HealthPercent);
	
	UFUNCTION(BlueprintCallable, Category = "Resurrection")
	void CancelResurrection();
	
	UFUNCTION(BlueprintPure, Category = "Resurrection")
	int32 GetRemainingRevives() const {return RemainingRevives;}
	
	UFUNCTION(BlueprintPure, Category = "Resurrection")
	int32 GetMaxRevives() const {return MaxRevives;}
	
	UFUNCTION(BlueprintPure, Category = "Resurrection")
	bool IsResurrectionInProgress() const {return bIsResurrectionInProgress;}
	
	UFUNCTION(BlueprintPure, Category = "Resurrection")
	float GetResurrectionTimeRemaining() const;
	
	// IRevivable 
	virtual bool CanBeRevived_Implementation() const override;
	virtual bool ReviveCharacter_Implementation(float HealthPercent) override;
	
	UPROPERTY(BlueprintAssignable, Category = "Resurrection|Events")
	FOnReviveCountChanged OnReviveCountChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Resurrection|Events")
	FOnResurrectionEvent OnResurrectionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Resurrection|Events")
	FOnResurrectionEvent OnResurrectionCancelled;

	UPROPERTY(BlueprintAssignable, Category = "Resurrection|Events")
	FOnResurrectionEvent OnResurrectionCompleted;

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resurrection", meta = (ClampMin = "0"))
	int32 MaxRevives = 3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resurrection", meta = (ClampMin = "0.0"))
	float DefaultResurrectionDuration = 5.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resurrection", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultResurrectionHealthPercent = 0.5f;

private:
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Resurrection", meta = (AllowPrivateAccess = true))
	int32 RemainingRevives = 3;
	
	bool bIsResurrectionInProgress = false;
	float PendingHealthPercent = 0.5f;
	FTimerHandle ResurrectionTimerHandle;
	
	void CompleteResurrection();
};
