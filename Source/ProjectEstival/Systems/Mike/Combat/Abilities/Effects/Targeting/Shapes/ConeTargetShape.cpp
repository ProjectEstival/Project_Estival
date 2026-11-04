#include "ConeTargetShape.h"
#include "DrawDebugHelpers.h"

bool UConeTargetShape::IsInside(const AActor* Actor, const FVector& Center, const FQuat& Rotation) const
{
	const FVector ToActor = (Actor->GetActorLocation() - Center).GetSafeNormal();
	if (ToActor.IsNearlyZero()) return true;
	
	const FVector Forward = Rotation.GetForwardVector().GetSafeNormal();
	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(Angle * 0.5f));
	return FVector::DotProduct(Forward, ToActor) >= CosHalfAngle;
}

void UConeTargetShape::DrawDebug(const UWorld* World, const FVector& Center, const FQuat& Rotation, const float Duration) const
{
	const FVector Forward = Rotation.GetForwardVector().GetSafeNormal();
	const float HalfAngle = Angle * 0.5f;
	const int32 Segments = FMath::Max(4, FMath::CeilToInt(Angle / 10.f));
	
	FVector Previous = Center + Forward.RotateAngleAxis(-HalfAngle, FVector::UpVector) * Radius;
	DrawDebugLine(World, Center, Previous, FColor::Red, false, Duration);
	
	for (int32 Index = 1; Index <= Segments; ++Index)
	{
		const float StepAngle = -HalfAngle + Angle * Index / Segments;
		const FVector Point = Center + Forward.RotateAngleAxis(StepAngle, FVector::UpVector) * Radius;
		DrawDebugLine(World, Previous, Point, FColor::Red, false, Duration);
		Previous = Point;
	}
	
	DrawDebugLine(World, Center, Previous, FColor::Red, false, Duration);
}