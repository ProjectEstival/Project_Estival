// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AC_LookAtComponent.generated.h"

/**
 *  Rotates the owner yaw toward a target.
 */
UCLASS(ClassGroup=(Estival), meta = (BlueprintSpawnableComponent))
class PROJECTESTIVAL_API UAC_LookAtComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAC_LookAtComponent();

	/** Rotation blend speed, 0 = instant */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="LookAt", meta = (ClampMin = 0))
	float InterpSpeed = 0.0f;

	/** Face a world location (mouse cursor, enemy...) */
	UFUNCTION(BlueprintCallable, Category="LookAt")
	void SetLookAtLocation(const FVector& Location);

	/** Face a direction (gamepad stick, AI...) */
	UFUNCTION(BlueprintCallable, Category="LookAt")
	void SetLookDirection(const FVector& Direction);

	/** Go back to facing the movement direction */
	UFUNCTION(BlueprintCallable, Category="LookAt")
	void ClearLookTarget();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	TOptional<float> TargetYaw;
};
