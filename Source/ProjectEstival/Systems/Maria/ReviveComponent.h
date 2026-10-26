#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReviveComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResurrectionEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLivesChanged, int32, RemainingLives);

UCLASS(ClassGroup = (Estival), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UReviveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UReviveComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Target side: this character is the one being revived. */
	UFUNCTION(BlueprintPure)
	bool CanBeRevived() const;

	UFUNCTION(BlueprintCallable)
	bool BeginResurrection(AActor* Reviver);

	UFUNCTION(BlueprintCallable)
	void CancelResurrection();

	UFUNCTION(BlueprintPure)
	bool IsInProgress() const { return bIsInProgress; }

	UFUNCTION(BlueprintPure)
	float GetProgress() const;

	UFUNCTION(BlueprintPure)
	int32 GetRemainingLives() const { return RemainingLives; }

	/** Reviver side: this character is the one holding the revive. */
	UFUNCTION(BlueprintCallable)
	bool StartReviveNearby();

	UFUNCTION(BlueprintCallable)
	void StopReviveNearby();

	UPROPERTY(BlueprintAssignable) FOnResurrectionEvent OnStarted;
	UPROPERTY(BlueprintAssignable) FOnResurrectionEvent OnCancelled;
	UPROPERTY(BlueprintAssignable) FOnResurrectionEvent OnCompleted;
	UPROPERTY(BlueprintAssignable) FOnLivesChanged OnLivesChanged;

protected:

	UPROPERTY(EditAnywhere, Category = "Resurrection", meta = (ClampMin = "0"))
	int32 MaxLives = 3;

	UPROPERTY(EditAnywhere, Category = "Resurrection", meta = (ClampMin = "0"))
	float Duration = 3.f;

	UPROPERTY(EditAnywhere, Category = "Resurrection", meta = (ClampMin = "0", ClampMax = "1"))
	float HealthPercent = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Resurrection", meta = (ClampMin = "0"))
	float ReviveRange = 200.f;

private:
	void Complete();
	UReviveComponent* FindReviveTarget() const;

	int32 RemainingLives = 0;
	bool bIsInProgress = false;
	TWeakObjectPtr<AActor> CurrentReviver;
	FTimerHandle TimerHandle;

	TWeakObjectPtr<UReviveComponent> ReviveTarget;
};