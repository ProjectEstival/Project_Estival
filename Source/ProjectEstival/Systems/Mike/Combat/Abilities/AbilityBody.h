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
};

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PROJECTESTIVAL_API UAbilityEffectPayload : public UObject
{
	GENERATED_BODY()
 
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	bool TryApply(const FAbilityContext& Context, AActor* Target) const;
	virtual bool TryApply_Implementation(const FAbilityContext& Context, AActor* Target) const { return true;}
};

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, CollapseCategories)
class PROJECTESTIVAL_API UAbilityPredicate : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, Category = "Ability")
	bool IsSatisfied(const FAbilityContext Context) const;
	virtual bool IsSatisfied_Implementation(const FAbilityContext& Context) const { return true;}
};