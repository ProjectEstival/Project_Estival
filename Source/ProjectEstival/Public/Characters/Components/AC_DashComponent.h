// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Public/Characters/Components/AC_ActionComponent.h"
#include "AC_DashComponent.generated.h"

/**
 *  Launches the character in its last movement input direction
 */
UCLASS(ClassGroup=(Estival), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UAC_DashComponent : public UAC_ActionComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dash", meta = (ClampMin = 0, ClampMax = 10000, Units = "cm/s"))
	float DashImpulse = 2500.0f;

protected:

	virtual void Execute() override;
};