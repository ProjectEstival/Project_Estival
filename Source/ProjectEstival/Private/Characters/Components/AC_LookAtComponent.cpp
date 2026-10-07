// Fill out your copyright notice in the Description page of Project Settings.

#include "Public/Characters/Components/AC_LookAtComponent.h"
#include "GameFramework/Actor.h"

UAC_LookAtComponent::UAC_LookAtComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAC_LookAtComponent::SetLookAtLocation(const FVector& Location)
{
	SetLookDirection(Location - GetOwner()->GetActorLocation());
}

void UAC_LookAtComponent::SetLookDirection(const FVector& Direction)
{
	if (!Direction.IsNearlyZero())
	{
		TargetYaw = Direction.Rotation().Yaw;
	}
}

void UAC_LookAtComponent::ClearLookTarget()
{
	TargetYaw.Reset();
}

void UAC_LookAtComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	const FRotator Current = Owner->GetActorRotation();

	//no explicit target face the movement direction
	float Yaw = Current.Yaw;
	if (TargetYaw.IsSet())
	{
		Yaw = TargetYaw.GetValue();
	}
	else if (Owner->GetVelocity().SizeSquared2D() > 1.0f)
	{
		Yaw = Owner->GetVelocity().Rotation().Yaw;
	}

	const FRotator Target(Current.Pitch, Yaw, Current.Roll);
	Owner->SetActorRotation(InterpSpeed > 0.0f ? FMath::RInterpTo(Current, Target, DeltaTime, InterpSpeed) : Target);
}