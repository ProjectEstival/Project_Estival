#pragma once
 
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "StatTypes.h"
#include "StatisticsComponent.generated.h"
 
class UStatModifierSet;
class UStatDefinitionSet;
 
// Fired whenever a stat's final value changes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStatChanged, FGameplayTag, Stat, float, NewValue, float, OldValue);
 
UCLASS(ClassGroup = (Stats), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UStatisticsComponent : public UActorComponent
{
	GENERATED_BODY()
 
public:
	UStatisticsComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	TObjectPtr<UStatDefinitionSet> DefaultStats;
 
	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnStatChanged OnStatChanged;
 
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void InitializeFromDefinitions(const UStatDefinitionSet* Definitions);
 
	UFUNCTION(BlueprintCallable, Category = "Stats",  meta = (Categories = "Stats."))
	void RegisterStat(FGameplayTag StatTag, const FStatDefinition& Definition);
 
	UFUNCTION(BlueprintPure, Category = "Stats",  meta = (Categories = "Stats."))
	bool HasStat(FGameplayTag Stat) const;
 
	UFUNCTION(BlueprintPure, Category = "Stats",  meta = (Categories = "Stats."))
	float GetFloat(FGameplayTag Stat, float Fallback = 0) const;
 
	UFUNCTION(BlueprintPure, Category = "Stats",  meta = (Categories = "Stats."))
	int32 GetInt(FGameplayTag Stat, int32 Fallback = 0) const;
	
	UFUNCTION(BlueprintCallable, Category = "Stats",  meta = (Categories = "Stats."))
	void AddModifier(FGameplayTag StatTag, FStatModifier Modifier, UObject* Source = nullptr);
	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddModifierSet(TMap<FGameplayTag, FStatModifier> Set, UObject* Source);
	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ApplyModifierAsset(UStatModifierSet* Set, UObject* Source);
	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 RemoveAllFromSource(UObject* Source);
	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ClearAllModifiers();
	
	virtual void InitializeComponent() override;
 
private:
	struct FStatChange
	{
		FGameplayTag Tag;
		float OldValue;
		float NewValue;
	};
	
	bool Recalculate(const FGameplayTag& Tag, FStatChange& OutChange);
	void BroadcastChanges(const TArray<FStatChange>& Changes) const;
 
	UPROPERTY(VisibleInstanceOnly, Category = "Stats|Debug")
	TMap<FGameplayTag, FStat> Stats;
};