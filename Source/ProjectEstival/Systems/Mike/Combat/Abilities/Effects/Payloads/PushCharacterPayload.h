// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "PushCharacterPayload.generated.h"

UENUM(BlueprintType)
enum class EPushDirection : uint8
{
	AwayFromOrigin,
	AlongDirection,
	TowardOrigin
};

/** Knocks the receiver away from the origin, along the direction or toward the origin. */
UCLASS(meta = (DisplayName = "Push"))
class PROJECTESTIVAL_API UPushCharacterPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = (Units = "cm"))
	float KnockbackDistance = 1000;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Knockback")
	EPushDirection PushDirection = EPushDirection::AwayFromOrigin;
	
	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};