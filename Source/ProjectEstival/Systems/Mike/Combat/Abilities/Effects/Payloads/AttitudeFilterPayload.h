// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/TeamAttitudeSolver.h"
#include "Systems/Mike/Combat/Abilities/AbilityBody.h"
#include "AttitudeFilterPayload.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTESTIVAL_API UAttitudeFilterPayload : public UAbilityEffectPayload
{
	GENERATED_BODY()
	
public:
	UAttitudeFilterPayload();
	
protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attitude")
	TSet<TEnumAsByte<ETeamAttitude::Type>> AllowedAttitudes;
	virtual bool TryApplySelf_Implementation(const FAbilityContext& Context, AActor* Target) const override;
};
