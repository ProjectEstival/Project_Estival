#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityDefinition.h"
#include "RuntimeAbilities.generated.h"

UENUM(BlueprintType)
enum class EAbilityPhase : uint8
{
	Idle,
	Windup,
	Recovery
};

USTRUCT()
struct FActiveAbility
{
	GENERATED_BODY()
	
	UPROPERTY()
	FGameplayTag Slot;
	
	UPROPERTY()
	TObjectPtr<UAbilityDefinition> Ability;
	
	EAbilityPhase Phase = EAbilityPhase::Idle;
	float PhaseStartTime = 0;
	float MontageEndTime = 0;
	FTimerHandle PhaseTimer;
};

USTRUCT()
struct FAbilitySlot
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleInstanceOnly, Category = "Slot")
	TObjectPtr<UAbilityDefinition> Ability;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Slot")
	float CooldownStart = 0;
	
	UPROPERTY(VisibleInstanceOnly, Category = "Slot")
	float CooldownEnd = 0;
};