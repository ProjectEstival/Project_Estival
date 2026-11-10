#pragma once
 
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "UObject/ObjectKey.h"
#include "StatTypes.h"
#include "StatisticsComponent.generated.h"
 
class UStatModifierSet;
class UStatDefinitionSet;

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
	void SetStatCollection(const UStatDefinitionSet* Definitions);
 
	UFUNCTION(BlueprintCallable, Category = "Stats",  meta = (Categories = "Stats."))
	void RegisterStat(FGameplayTag StatTag, const FStatDefinition& Definition);
 
	UFUNCTION(BlueprintPure, Category = "Stats",  meta = (Categories = "Stats."))
	bool HasStat(FGameplayTag Stat) const;
 
	UFUNCTION(BlueprintPure, Category = "Stats",  meta = (Categories = "Stats."))
	float GetFloat(FGameplayTag Stat, float Fallback = 0) const;
 
	UFUNCTION(BlueprintPure, Category = "Stats",  meta = (Categories = "Stats."))
	int32 GetInt(FGameplayTag Stat, int32 Fallback = 0) const;
	
	UFUNCTION(BlueprintCallable, Category = "Stats",  meta = (Categories = "Stats."))
	void AddModifier(FGameplayTag StatTag, FStatModifier Modifier, const UObject* Source = nullptr);
	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddModifierSet(TMap<FGameplayTag, FStatModifier> Set, const UObject* Source);
	
	/** Adds modifiers that remove themselves after Duration (0 = until removed). Adding again from the same source replaces them and restarts the timer. */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddTimedModifierSet(TMap<FGameplayTag, FStatModifier> Set, const UObject* Source, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ApplyModifierAsset(UStatModifierSet* Set, const UObject* Source);
	
	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 RemoveAllFromSource(const UObject* Source);
	
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
	
	UPROPERTY(Transient)
	TSet<FGameplayTag> RegisteredStats;

	TMap<TObjectKey<UObject>, FTimerHandle> SourceTimers;
};