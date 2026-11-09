#include "TargetShape.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

void UTargetShape::CollectActors(const UWorld* World, const FVector& Center, const FQuat& Rotation, const ECollisionChannel Channel,
	const FCollisionQueryParams& Params, TArray<AActor*>& OutActors) const
{
	if (!World) return;
	
	TArray<FOverlapResult> Overlaps;
	const FVector ShapeCenter = Center + Rotation.RotateVector(GetLocalOffset());
	World->OverlapMultiByChannel(Overlaps, ShapeCenter, Rotation, Channel, GetCollisionShape(), Params);
	
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (AActor* Actor = Overlap.GetActor(); IsValid(Actor) && IsInside(Actor, Center, Rotation)) OutActors.AddUnique(Actor);
	}
}