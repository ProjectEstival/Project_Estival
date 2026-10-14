#pragma once
 
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Systems/Mike/Combat/Abilities/AbilityDefinition.h"
#include "Systems/Mike/Combat/ResourceComponent.h"
#include "LoadoutComponent.generated.h"
 
class ULoadout;
class UStatisticsComponent;
struct FAbilityContext;
 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadoutChanged, ULoadout*, OldLoadout, ULoadout*, NewLoadout);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityActivated, FGameplayTag, Slot, UAbilityDefinition*, Ability);
 
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API ULoadoutComponent : public UActorComponent
{
	GENERATED_BODY()
 
public:
	ULoadoutComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TObjectPtr<ULoadout> DefaultLoadout;
 
	UPROPERTY(BlueprintAssignable, Category = "Loadout")
	FOnLoadoutChanged OnLoadoutChanged;
 
	UPROPERTY(BlueprintAssignable, Category = "Loadout")
	FOnAbilityActivated OnAbilityActivated;
	
	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void SetLoadout(ULoadout* NewLoadout);
 
	UFUNCTION(BlueprintPure, Category = "Loadout")
	ULoadout* GetLoadout() const { return CurrentLoadout; }
	
	UFUNCTION(BlueprintCallable, Category = "Loadout", meta = (Categories = "Ability."))
	bool TryActivate(FGameplayTag Slot);
 
	UFUNCTION(BlueprintPure, Category = "Loadout", meta = (Categories = "Ability."))
	UAbilityDefinition* GetAbility(FGameplayTag Slot) const;
 
	UFUNCTION(BlueprintPure, Category = "Loadout", meta = (Categories = "Ability."))
	bool IsOnCooldown(FGameplayTag Slot) const;
 
	UFUNCTION(BlueprintPure, Category = "Loadout", meta = (Categories = "Ability."))
	float GetCooldownRemaining(FGameplayTag Slot) const;
	
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	bool bConsumesResources = true;
 
private:
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
 
	// world time in seconds for when a slot becomes usable again.
	TMap<FGameplayTag, float> CooldownEnds;
};