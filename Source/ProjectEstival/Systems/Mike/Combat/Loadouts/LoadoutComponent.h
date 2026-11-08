#pragma once
 
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Systems/Mike/Combat/Abilities/AbilityDefinition.h"
#include "RuntimeAbilities.h"
#include "Systems/Mike/Combat/ResourceComponent.h"
#include "LoadoutComponent.generated.h"
 
class ULoadout;
class UStatisticsComponent;
struct FAbilityContext;

 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadoutChanged, ULoadout*, OldLoadout, ULoadout*, NewLoadout);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityEvent, FGameplayTag, Slot, UAbilityDefinition*, Ability);
 
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API ULoadoutComponent : public UActorComponent
{
	GENERATED_BODY()
 
public:
	ULoadoutComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TObjectPtr<ULoadout> DefaultLoadout;
 
	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnLoadoutChanged OnLoadoutChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnAbilityEvent OnAbilityActivated;
	
	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnAbilityEvent OnAbilityCommitted;
	
	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnAbilityEvent OnAbilityInterrupted;
	
	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnAbilityEvent OnAbilityEnded;
	
	UPROPERTY(BlueprintAssignable, Category = "Loadout|Events")
	FOnAbilityEvent OnSlotChanged;
	
	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void SetLoadout(ULoadout* NewLoadout);
 
	UFUNCTION(BlueprintPure, Category = "Loadout")
	ULoadout* GetLoadout() const { return CurrentLoadout; }
	
	UFUNCTION(BlueprintCallable, Category = "Loadout", meta = (Categories = "Ability."))
	bool TryActivate(FGameplayTag Slot);
	
	UFUNCTION(BlueprintCallable, Category = "Loadout")
	bool Interrupt();
	
	void AddInterruptibleTimer(const FTimerHandle& Handle);
 
	UFUNCTION(BlueprintPure, Category = "Loadout", meta = (Categories = "Ability."))
	UAbilityDefinition* GetAbility(FGameplayTag Slot) const;
	
	UFUNCTION(BlueprintCallable, Category = "Loadout|Slots", meta = (Categories = "Ability."))
	void EquipAbility(FGameplayTag Slot, UAbilityDefinition* Ability);
 
	UFUNCTION(BlueprintPure, Category = "Loadout", meta = (Categories = "Ability."))
	bool IsOnCooldown(FGameplayTag Slot) const;
 
	UFUNCTION(BlueprintPure, Category = "Loadout", meta = (Categories = "Ability."))
	float GetCooldownRemaining(FGameplayTag Slot) const;
	
	UFUNCTION(BlueprintPure, Category = "Loadout", meta = (Categories = "Ability."))
	float GetCooldownRemaining01(FGameplayTag Slot) const;
	
	UFUNCTION(BlueprintCallable, Category = "Loadout|Slots", meta = (Categories = "Ability."))
	void ReduceCooldown(FGameplayTag Slot, float Seconds);
	
	UFUNCTION(BlueprintPure, Category = "Loadout")
	bool IsBusy() const { return Active.Phase != EAbilityPhase::Idle; }
	
	UFUNCTION(BlueprintPure, Category = "Loadout")
	EAbilityPhase GetPhase() const { return Active.Phase; }
	
	UFUNCTION(BlueprintPure, Category = "Loadout")
	FGameplayTag GetActiveSlot() const { return Active.Slot; }
	
	UFUNCTION(BlueprintPure, Category = "Loadout")
	float GetCastProgress01() const;
	
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	bool bConsumesResources = true;
 
private:
	void Commit();
	void EndActive(bool bInterrupted);
	void RemoveCastingModifiers();
	
	bool CanAffordCosts(const UAbilityDefinition* Ability);
	void SpendCosts(const UAbilityDefinition* Ability);
	
	UStatisticsComponent* ResolveStats();
	UResourceComponent* ResolveResources();
	FAbilityContext BuildContext(UAbilityDefinition* Ability);
	static void ExecuteEffects(const FAbilityContext& Context);

	UPROPERTY(Transient)
	TObjectPtr<ULoadout> CurrentLoadout;
 
	UPROPERTY(Transient)
	TObjectPtr<UStatisticsComponent> StatisticsComponent;
	
	UPROPERTY(Transient)
	TObjectPtr<UResourceComponent> ResourceComponent;

	UPROPERTY(Transient)
	FActiveAbility Active;
 
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Loadout|Debug")
	TMap<FGameplayTag, FAbilitySlot> Slots;
};