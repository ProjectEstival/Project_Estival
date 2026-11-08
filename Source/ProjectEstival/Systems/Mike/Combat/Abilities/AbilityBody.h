#pragma once
 
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AbilityContext.h"
#include "AbilityBody.generated.h"
 
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PROJECTESTIVAL_API UAbilityEffect : public UObject
{
	GENERATED_BODY()
 
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	void Execute(const FAbilityContext& Context) const;
	virtual void Execute_Implementation(const FAbilityContext& Context) const {}

	template <typename ArrayType>
	static void ExecuteAll(const ArrayType& Effects, const FAbilityContext& Context)
	{
		for (UAbilityEffect* Effect : Effects)
		{
			if (Effect) Effect->Execute(Context);
		}
	}
};

UENUM()
enum EPayloadSuccess : uint8
{
	SelfOnly,
	CompleteChain,
	ChainPartial
};

UENUM(BlueprintType)
enum class ETargetOrCaster : uint8
{
	Target,
	Caster
};

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PROJECTESTIVAL_API UAbilityEffectPayload : public UObject
{
	GENERATED_BODY()
 
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	ETargetOrCaster ApplyTo = ETargetOrCaster::Target;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	TEnumAsByte<EPayloadSuccess> SuccessRequirement = ChainPartial;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	bool TryApplySelf(const FAbilityContext& Context, AActor* Target) const;
	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const { return true;}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ability")
	TArray<UAbilityEffectPayload*> SuccessChain;
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	bool TryApply(const FAbilityContext& Context, AActor* Target)
	{
		AActor* Receiver = ApplyTo == ETargetOrCaster::Caster ? Context.Instigator.Get() : Target;
		if (!TryApplySelf(Context, Receiver)) return false;
		
		int SuccessfulChains = 0;
		const int TotalChains = SuccessChain.Num();
		
		for (const auto ChainPayload : SuccessChain)
		{
			if (!ChainPayload) continue;
			if (ChainPayload->TryApply(Context, Target)) SuccessfulChains++;
		}
		
		switch (SuccessRequirement)
		{
			case SelfOnly: return true;
			case CompleteChain: return SuccessfulChains == TotalChains;
			case ChainPartial: return TotalChains == 0 || SuccessfulChains > 0;
			default: return false;
		}
	}

	template <typename ArrayType>
	static bool ApplyAll(const ArrayType& Payloads, const FAbilityContext& Context, AActor* Target)
	{
		bool bAnyApplied = false;

		for (UAbilityEffectPayload* Payload : Payloads)
		{
			if (IsValid(Payload) && Payload->TryApply(Context, Target)) bAnyApplied = true;
		}

		return bAnyApplied;
	}
};

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PROJECTESTIVAL_API UAbilityPredicate : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	bool IsSatisfied(const FAbilityContext& Context) const;
	virtual bool IsSatisfied_Implementation(const FAbilityContext& Context) const { return true;}
};