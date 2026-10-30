#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ResourceComponent.generated.h"

class UStatisticsComponent;

USTRUCT(BlueprintType)
struct FResourceConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	bool bStartFull = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
	float RegenPerSecond = 0.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnResourceChanged, FGameplayTag, Stat, float, NewValue, float, OldValue, float, MaxValue);

UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UResourceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UResourceComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resources", meta = (Categories = "Stats.Resources."))
	TMap<FGameplayTag, FResourceConfig> ResourceStats;

	UPROPERTY(BlueprintAssignable, Category = "Resources")
	FOnResourceChanged OnResourceChanged;
	
	UFUNCTION(BlueprintPure, Category = "Resources", meta = (Categories = "Stats.Resources."))
	bool HasResource(FGameplayTag StatTag) const;
	
	UFUNCTION(BlueprintPure, Category = "Resources", meta = (Categories = "Stats.Resources."))
	float GetFloat(FGameplayTag StatTag) const;

	UFUNCTION(BlueprintPure, Category = "Resources", meta = (Categories = "Stats.Resources."))
	int32 GetInt(FGameplayTag StatTag) const;
	
	UFUNCTION(BlueprintPure, Category = "Resources", meta = (Categories = "Stats.Resources."))
	float GetMax(FGameplayTag StatTag) const;
	
	UFUNCTION(BlueprintPure, Category = "Resources", meta = (Categories = "Stats.Resources."))
	float GetPercentage01(FGameplayTag StatTag) const;

	UFUNCTION(BlueprintPure, Category = "Resources", meta = (Categories = "Stats.Resources."))
	bool CanAfford(FGameplayTag StatTag, float Amount) const;
	
	UFUNCTION(BlueprintCallable, Category = "Resources", meta = (Categories = "Stats.Resources."))
	float ModifyResource(FGameplayTag StatTag, float Amount);
	
	UFUNCTION(BlueprintCallable, Category = "Resources", meta = (Categories = "Stats.Resources."))
	bool TrySpend(FGameplayTag StatTag, float Amount);

	UFUNCTION(BlueprintCallable, Category = "Resources", meta = (Categories = "Stats.Resources."))
	void Fill(FGameplayTag StatTag);
	
	UFUNCTION(BlueprintCallable, Category = "Resources", meta = (Categories = "Stats.Resources."))
	void SetCurrent(FGameplayTag StatTag, float Value);
	
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	const FResourceConfig* FindConfig(FGameplayTag StatTag) const;
	float Write(FGameplayTag StatTag, float NewValue);

	UFUNCTION()
	void HandleStatChanged(FGameplayTag Stat, float NewValue, float OldValue);

	UPROPERTY(Transient)
	TObjectPtr<UStatisticsComponent> Statistics;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Resources|Debug")
	TMap<FGameplayTag, float> Resources;
};