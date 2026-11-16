#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Systems/Maria/Vendor Interaction System/Interactable.h"
#include "ReviveComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResurrectionEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLivesChanged, int32, RemainingLives);

UCLASS(ClassGroup = (Estival), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UReviveComponent : public UActorComponent, public IInteractable
{
	GENERATED_BODY()

public:

#pragma region Public Methods
	
	UReviveComponent();
	virtual void BeginPlay() override;
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual float GetHoldDuration_Implementation() const override { return Duration; }
	virtual bool TryInteract_Implementation(AActor* Interactor) override;
	virtual void OnHoldStarted_Implementation(AActor* Interactor) override {OnStarted.Broadcast();}
	virtual void OnHoldCancelled_Implementation(AActor* Interactor) override {OnCancelled.Broadcast();}
#pragma endregion

#pragma region Target Side
	/** Target side: this character is the one being revived. */
	UFUNCTION(BlueprintPure)
	bool CanBeRevived() const;

	UFUNCTION(BlueprintPure)
	int32 GetRemainingLives() const { return RemainingLives; }
#pragma endregion

#pragma region Events
	UPROPERTY(BlueprintAssignable) FOnResurrectionEvent OnStarted;
	UPROPERTY(BlueprintAssignable) FOnResurrectionEvent OnCancelled;
	UPROPERTY(BlueprintAssignable) FOnResurrectionEvent OnCompleted;
	UPROPERTY(BlueprintAssignable) FOnLivesChanged OnLivesChanged;
#pragma endregion

protected:

#pragma region Serialized Fields
	UPROPERTY(EditAnywhere, Category = "Resurrection", meta = (ClampMin = "0"))
	int32 MaxLives = 3;

	UPROPERTY(EditAnywhere, Category = "Resurrection", meta = (ClampMin = "0"))
	float Duration = 3.f;

	UPROPERTY(EditAnywhere, Category = "Resurrection", meta = (ClampMin = "0", ClampMax = "1"))
	float HealthPercent = 0.5f;
#pragma endregion

private:

#pragma region Private Fields
	int32 RemainingLives = 0;
	bool bIsInProgress = false;
	TWeakObjectPtr<AActor> CurrentReviver;
	FTimerHandle TimerHandle;

	TWeakObjectPtr<UReviveComponent> ReviveTarget;
#pragma endregion
};