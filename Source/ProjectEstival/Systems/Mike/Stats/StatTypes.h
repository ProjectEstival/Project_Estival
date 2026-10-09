#pragma once
 
#include "CoreMinimal.h"
#include "StatTypes.generated.h"

USTRUCT(BlueprintType)
struct FStatModifier
{
	GENERATED_BODY()
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Value = 0;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool bAsPercentage = false;
};

USTRUCT()
struct FActiveStatModifier
{
	GENERATED_BODY()
 
	UPROPERTY(VisibleAnywhere, Category = "Stats")
	FStatModifier Modifier;
 
	UPROPERTY(VisibleAnywhere, Category = "Stats")
	TObjectPtr<UObject> Source = nullptr;
};
 
USTRUCT(BlueprintType)
struct FStatDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BaseValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Min = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Max = 100;
};

USTRUCT(BlueprintType)
struct FStat : public FStatDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Stats|Runtime")
	float CachedValue = 0;

	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Stats|Runtime")
	TArray<FActiveStatModifier> Modifiers;

	float Calculate() const
	{
		float FlatSum = 0, PercentSum = 0;
		
		for (const FActiveStatModifier& Active : Modifiers)
		{
			if (Active.Modifier.bAsPercentage) PercentSum += Active.Modifier.Value;
			else FlatSum += Active.Modifier.Value;
		}

		const float Result = (BaseValue + FlatSum) * (1 + PercentSum / 100);
		return FMath::Clamp(Result, Min, Max);
	}
	
	void SetupFromDef(const FStatDefinition StatDef)
	{
		Max = StatDef.Max;
		Min = StatDef.Min;
		BaseValue = FMath::Clamp(StatDef.BaseValue, Min, Max);
		CachedValue = Calculate();
	}
};

// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UStatModifierProvider : public UInterface
{
	GENERATED_BODY()
};

class PROJECTESTIVAL_API IStatModifierProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Stats")
	TMap<FGameplayTag, FStatModifier> GetModifiers() const;
};