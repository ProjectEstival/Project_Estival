#pragma once
 
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Systems/Mike/Combat/Abilities/AbilityDefinition.h"
#include "RuntimeAbilities.h"
#include "Systems/Mike/Combat/ResourceComponent.h"
#include "Systems/Mike/Combat/Abilities/AbilityContext.h"
#include "Systems/Core/TagDefinitions.h"
#include "LoadoutComponent.generated.h"
 
class ULoadout;
class UStatisticsComponent;
class UAbilityTask;
class UAimComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadoutChanged, ULoadout*, OldLoadout, ULoadout*, NewLoadout);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityStateChanged, FGameplayTag, Slot, UAbilityDefinition*, Ability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityEvent, FGameplayTag, Event);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API ULoadoutComponent : public UActorComponent
{
	GENERATED_BODY()
 
public:
	ULoadoutComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TObjectPtr<ULoadout> DefaultLoadout;
 
	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnLoadoutChanged OnLoadoutChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnAbilityStateChanged OnSlotChanged;

	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnAbilityStateChanged OnAbilityActivated;

	/** The ability paid its costs and started its cooldown. */
	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnAbilityStateChanged OnAbilityCommitted;

	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnAbilityStateChanged OnAbilityInterrupted;

	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnAbilityStateChanged OnAbilityEnded;

	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnAbilityEvent OnAbilityEvent;

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void SetLoadout(ULoadout* NewLoadout);
 
	UFUNCTION(BlueprintPure, Category = "Loadout")
	ULoadout* GetLoadout() const { return CurrentLoadout; }

	UFUNCTION(BlueprintCallable, Category = "Loadout|Slots", meta = (Categories = "Ability."))
	void EquipAbility(FGameplayTag Slot, UAbilityDefinition* Ability);

	UFUNCTION(BlueprintPure, Category = "Loadout|Slots", meta = (Categories = "Ability."))
	UAbilityDefinition* GetAbility(FGameplayTag Slot) const;

	UFUNCTION(BlueprintPure, Category = "Loadout|Slots", meta = (Categories = "Ability."))
	bool HasFollowUp(FGameplayTag Slot) const;

	UFUNCTION(BlueprintPure, Category = "Loadout|Cooldowns", meta = (Categories = "Ability."))
	bool IsOnCooldown(FGameplayTag Slot) const;

	UFUNCTION(BlueprintPure, Category = "Loadout|Cooldowns", meta = (Categories = "Ability."))
	float GetCooldownRemaining(FGameplayTag Slot) const;

	UFUNCTION(BlueprintPure, Category = "Loadout|Cooldowns", meta = (Categories = "Ability."))
	float GetCooldownRemaining01(FGameplayTag Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Loadout|Cooldowns", meta = (Categories = "Ability."))
	void ReduceCooldown(FGameplayTag Slot, float Seconds);
	
	UFUNCTION(BlueprintPure, Category = "Loadout", meta = (Categories = "Ability."))
	bool CanActivate(FGameplayTag Slot);

	UFUNCTION(BlueprintCallable, Category = "Loadout", meta = (Categories = "Ability."))
	bool TryActivate(FGameplayTag Slot);

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	bool Interrupt();

	UFUNCTION(BlueprintPure, Category = "Loadout")
	bool IsBusy() const { return Active != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Loadout")
	UAbilityDefinition* GetActiveAbility() const { return Active ? Active->GetAbility() : nullptr; }

	UFUNCTION(BlueprintPure, Category = "Loadout")
	FGameplayTag GetActiveSlot() const { return Active ? Active->GetSlot() : FGameplayTag(); }

	UFUNCTION(BlueprintPure, Category = "Loadout")
	float GetCastProgress01() const;

	UFUNCTION(BlueprintCallable, Category = "Loadout|Input", meta = (Categories = "Ability."))
	bool InputPressed(FGameplayTag Slot);

	UFUNCTION(BlueprintCallable, Category = "Loadout|Input", meta = (Categories = "Ability."))
	void InputReleased(FGameplayTag Slot);

	UFUNCTION(BlueprintPure, Category = "Loadout|Input", meta = (Categories = "Ability."))
	bool IsSlotHeld(FGameplayTag Slot) const { return HeldSlots.Contains(Slot); }

	/** Sends an event to the active ability's tasks and to everything listening to OnAbilityEvent.*/
	UFUNCTION(BlueprintCallable, Category = "Loadout|Input", meta = (Categories = "AbilityEvent"))
	void SendAbilityEvent(FGameplayTag Event);

	/** Where this character is aiming. */
	UFUNCTION(BlueprintPure, Category = "Loadout|Aim")
	UAimComponent* GetAim();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	bool bConsumesResources = true;
 
	UPROPERTY(EditAnywhere, Category = "Loadout|Debug")
	bool bPrintDebug = false;

private:
	friend class UAbilityActivation;
	friend class UAbilityTask;

	FAbilityContext MakeContext(UAbilityDefinition* Ability, UAbilityActivation* Activation = nullptr);
	UStatisticsComponent* GetStats();
	float GetAttackSpeed();
	bool TryCommit(FGameplayTag Slot, const UAbilityDefinition* Ability, bool bFromFollowUp);
	void HandleActivationEnded(UAbilityActivation* Activation, bool bInterrupted);

	void AddTask(UAbilityTask* Task);
	void RemoveTask(UAbilityTask* Task);
	void CancelTasks(const UAbilityActivation* Activation);
	bool HasTaskKeepingActive(const UAbilityActivation* Activation) const;

	/** Why Slot can't activate right now. Empty if it can. */
	FString GetBlockReason(FGameplayTag Slot);

	void SetFollowUp(FGameplayTag Slot, UAbilityDefinition* FollowUp);
	void ClearFollowUp(FGameplayTag Slot, bool bBroadcast);
	void StartFollowUpWindow(FGameplayTag Slot, float Window);
	void RepeatIfHeld(FGameplayTag Slot);

	UResourceComponent* GetResources();
	bool CanAffordCosts(const UAbilityDefinition* Ability);
	void SpendCosts(const UAbilityDefinition* Ability);
	float GetCooldownMultiplier();

	/** Calls Function for each running task of Activation. Safe if tasks end during the loop. */
	void ForEachTask(const UAbilityActivation* Activation, TFunctionRef<void(UAbilityTask&)> Function);

	UPROPERTY(Transient)
	TObjectPtr<ULoadout> CurrentLoadout;
 
	UPROPERTY(Transient)
	TObjectPtr<UStatisticsComponent> StatisticsComponent;
	
	UPROPERTY(Transient)
	TObjectPtr<UResourceComponent> ResourceComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAimComponent> AimComponent;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityActivation> Active;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask>> Tasks;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Loadout|Debug")
	TMap<FGameplayTag, FAbilitySlot> Slots;

	TSet<FGameplayTag> HeldSlots;
};