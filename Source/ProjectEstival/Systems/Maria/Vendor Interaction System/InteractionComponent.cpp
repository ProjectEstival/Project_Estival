// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionComponent.h"
#include "Interactable.h"
#include "Systems/Core/Characters/CharacterBase.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"


// Sets default values for this component's properties
UInteractionComponent::UInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopInteract();
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!IsHoldValid())
	{
		StopInteract();
		return;
	}
	
	HoldElapsed += DeltaTime;
	if (HoldElapsed < HoldDuration) return;
	
	UObject* Target = HoldTarget.Get();
	ResetHold();
	IInteractable::Execute_Interact(Target, GetOwner());
	OnInteracted.Broadcast();
}

bool UInteractionComponent::StartInteract()
{
	const AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(GetOwner());
	if (!Character || !Character->IsAlive() || HoldTarget.IsValid()) return false;
	
	UObject* Target = FindNearestInteractable();
	if (!Target) return false;
	
	const float Duration = IInteractable::Execute_GetHoldDuration(Target);
	if (Duration <= 0.f)
	{
		IInteractable::Execute_Interact(Target, GetOwner());
		OnInteracted.Broadcast();
		return true;
	}
	
	HoldTarget = Target;
	HoldDuration = Duration;
	HoldElapsed = 0.f;
	SetComponentTickEnabled(true);
	
	IInteractable::Execute_OnHoldStarted(Target, GetOwner());
	OnHoldStarted.Broadcast();
	return true;
}

void UInteractionComponent::StopInteract()
{
	UObject* Target = HoldTarget.Get();
	if (!Target)
	{
		SetComponentTickEnabled(false);
		return;
	}
	
	ResetHold();
	IInteractable::Execute_OnHoldCancelled(Target, GetOwner());
	OnHoldCancelled.Broadcast();
}

float UInteractionComponent::GetHoldProgress() const
{
	if (!HoldTarget.IsValid() || HoldDuration <= 0.f) return 0.f;
	return FMath::Clamp(HoldElapsed/HoldDuration, 0.f, 1.f);
}

UObject* UInteractionComponent::FindNearestInteractable()const
{
	if (!GetWorld()) return nullptr;
	
	const FVector Origin = GetOwner()->GetActorLocation();
	
	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Origin,
		FQuat::Identity, InteractChannel,
		FCollisionShape::MakeSphere(InteractRange),
		FCollisionQueryParams(SCENE_QUERY_STAT(FindInteractable),
			false,
			GetOwner()));
	
	UObject* Best = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	
	auto Consider = [&](UObject* Candidate, const AActor* Actor)
	{
		if (!IInteractable::Execute_CanInteract(Candidate, GetOwner())) return;
		
		const float DistSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Candidate;
		}
	};
	
	for (const FOverlapResult& Overlap:Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!Actor) continue;
		
		if (Actor->Implements<UInteractable>()) Consider(Actor, Actor);
		
		for (UActorComponent* Component : Actor->GetComponentsByInterface(UInteractable::StaticClass())) Consider(Component, Actor);
	}
	return Best;
}

AActor* UInteractionComponent::GetTargetActor(const UObject* Target) const
{
	if (const AActor* Actor = Cast<AActor>(Target)) return const_cast<AActor*>(Actor);
	if (const UActorComponent* Component = Cast<UActorComponent>(Target)) return Component->GetOwner();
	return nullptr;
}

bool UInteractionComponent::IsHoldValid() const
{
	const AEstivalBaseCharacter* Character = Cast<AEstivalBaseCharacter>(GetOwner());
	UObject* Target = HoldTarget.Get();
	const AActor* TargetActor = GetTargetActor(Target);
	
	return Character && Character->IsAlive() && Target && TargetActor
	&& FVector::DistSquared(Character->GetActorLocation(), TargetActor->GetActorLocation()) <= FMath::Square(InteractRange)
	&& IInteractable::Execute_CanInteract(Target, GetOwner());
}

void UInteractionComponent::ResetHold()
{
	HoldTarget.Reset();
	HoldElapsed = 0.f;
	HoldDuration = 0.f;
	SetComponentTickEnabled(false);
}