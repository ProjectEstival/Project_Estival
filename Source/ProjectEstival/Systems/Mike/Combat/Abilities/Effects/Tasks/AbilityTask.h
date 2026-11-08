#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "AbilityTask.generated.h"

class UAbilityActivation;
class ULoadoutComponent;

UENUM(BlueprintType)
enum class EAbilityTaskLifetime : uint8
{
	/** The ability stays active until this task ends. Interrupts cancel it. */
	KeepAbilityActive,
	/** Stops when the ability ends. Interrupts cancel it. */
	EndWithAbility,
	/** Keeps running after the ability ends. Interrupts don't cancel it. */
	Background
};

/**
 * An effect that takes time. In Blueprint, start your logic from On Start and react with the other On events.
 */
UCLASS(Abstract, Blueprintable, HideFunctions = (Execute))
class PROJECTESTIVAL_API UAbilityTask : public UAbilityEffect
{
	GENERATED_BODY()

public:
	virtual void Execute_Implementation(const FAbilityContext& InContext) const override final;
	virtual UWorld* GetWorld() const override;

	/** Ends the task now. */
	UFUNCTION(BlueprintCallable, Category = "Task")
	void Finish();
	
	void Cancel();
	void HandleInput(bool bPressed);
	void HandleEvent(FGameplayTag Event);
	bool IsRunning() const { return bRunning; }
	EAbilityTaskLifetime GetLifetime() const { return Lifetime; }
	UAbilityActivation* GetActivation() const { return Context.Activation.Get(); }

	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Task")
	float GetProgress() const;
	virtual float GetProgress_Implementation() const { return -1.f; }

protected:
	UPROPERTY()
	bool bLifetimeEditable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task", meta = (EditCondition = "bLifetimeEditable", EditConditionHides, HideEditConditionToggle))
	EAbilityTaskLifetime Lifetime = EAbilityTaskLifetime::EndWithAbility;

	/** Spawned when the task starts and removed when it ends. While the task follows the caster, it stays on them and points where the effects will go. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Task")
	TSubclassOf<AActor> Indicator;

	UFUNCTION(BlueprintNativeEvent, Category = "Task")
	void OnStart();
	virtual void OnStart_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Task")
	void OnInputPressed();
	virtual void OnInputPressed_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Task")
	void OnInputReleased();
	virtual void OnInputReleased_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Task")
	void OnAbilityEvent(FGameplayTag Event);
	virtual void OnAbilityEvent_Implementation(FGameplayTag Event) {}

	UFUNCTION(BlueprintNativeEvent, Category = "Task")
	void OnEnd(bool bCancelled);
	virtual void OnEnd_Implementation(bool bCancelled) {}

	UFUNCTION(BlueprintPure, Category = "Task")
	FAbilityContext GetContext() const;

	UFUNCTION(BlueprintCallable, Category = "Task")
	void RunEffects(const TArray<UAbilityEffect*>& Effects) const;

	UFUNCTION(BlueprintPure, Category = "Task")
	bool IsInputHeld() const;

	UFUNCTION(BlueprintPure, Category = "Task")
	float GetElapsedTime() const;

	UFUNCTION(BlueprintCallable, Category = "Task")
	bool CommitAbility() const;

	UFUNCTION(BlueprintPure, Category = "Task")
	AActor* GetIndicator() const { return SpawnedIndicator; }

	ULoadoutComponent* GetLoadout() const { return Context.Loadout; }

	/** Runs every frame while the indicator follows the caster. */
	virtual void UpdateIndicator() const;

private:
	void Start(const FAbilityContext& InContext);
	void End(bool bCancelled);
	void SpawnIndicator();

	UPROPERTY(Transient)
	FAbilityContext Context;

	UPROPERTY(Transient)
	TObjectPtr<AActor> SpawnedIndicator;

	float StartTime = 0.f;
	bool bRunning = false;
};
