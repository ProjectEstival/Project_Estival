#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Abstract/TargetShape.h"
#include "SphereTargetShape.generated.h"

/** A sphere around the origin. */
UCLASS(meta = (DisplayName = "Sphere"))
class PROJECTESTIVAL_API USphereTargetShape : public UTargetShape
{
	GENERATED_BODY()
	
public:
	virtual void DrawDebug(const UWorld* World, const FVector& Center, const FQuat& Rotation, float Duration) const override
	{
		DrawDebugSphere(World, Center, Radius, 16, FColor::Red, false, Duration);
	}
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape", meta = (ClampMin = 1, Units = "cm"))
	float Radius = 200.f;
	
	virtual FCollisionShape GetCollisionShape() const override
	{
		return FCollisionShape::MakeSphere(Radius);
	}
};