// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abstract/TargetingEffect.h"
#include "MeleeTargetingEffect.generated.h"

UCLASS()
class PROJECTESTIVAL_API UMeleeTargetingEffect : public UTargetingEffect
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrawDebug = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0.f, Units = "cm"))
	float RangeReach = 150;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = .1f, Units = "cm"))
	float HitboxWidth = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Pawn;
	
public:
	virtual TSet<AActor*> GetTargets_Implementation(const FAbilityContext& Context) const override;
};