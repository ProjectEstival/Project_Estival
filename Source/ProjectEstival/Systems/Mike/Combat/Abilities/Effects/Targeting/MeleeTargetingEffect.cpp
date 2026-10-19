// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeTargetingEffect.h"
#include "Engine/OverlapResult.h"

TSet<AActor*> UMeleeTargetingEffect::GetTargets_Implementation(const FAbilityContext& Context) const
{
	TSet<AActor*> Targets;
	const AActor* Instigator = Context.Instigator;
	const UWorld* World = GetWorld();

	if (!IsValid(Instigator) || !World) return Targets;
	
	const FVector Center = Instigator->GetActorLocation() + Instigator->GetActorForwardVector() * (RangeReach * 0.5f);
	const FVector HalfExtent(RangeReach * 0.5f, HitboxWidth * 0.5f, 100.f);
	const FQuat Rotation = Instigator->GetActorRotation().Quaternion();

	TArray<FOverlapResult> Overlaps;

	const FCollisionQueryParams Params(
		SCENE_QUERY_STAT(AbilityMelee),
		false,
		Instigator
	);

	World->OverlapMultiByChannel(
		Overlaps,
		Center,
		Rotation,
		CollisionChannel,
		FCollisionShape::MakeBox(HalfExtent),
		Params
	);

	if (bDrawDebug) DrawDebugBox(World, Center, HalfExtent, Rotation, FColor::Red, false, 0.5f);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!IsValid(Target) || Target == Instigator) continue;
		Targets.Add(Target);
	}

	return Targets;
}
