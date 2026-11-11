#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Abstract/TargetShape.h"
#include "ConeTargetShape.generated.h"

/** A cone in front of the origin, Angle wide. */
UCLASS(meta = (DisplayName = "Cone"))
class PROJECTESTIVAL_API UConeTargetShape : public UTargetShape
{
	GENERATED_BODY()
	
public:
	virtual void DrawDebug(const UWorld* World, const FVector& Center, const FQuat& Rotation, float Duration) const override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape", meta = (ClampMin = 1, Units = "cm"))
	float Radius = 200;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape", meta = (ClampMin = 1, ClampMax = 360, Units = "deg"))
	float Angle = 90;
	
	virtual FCollisionShape GetCollisionShape() const override
	{
		return FCollisionShape::MakeSphere(Radius);
	}
	virtual bool IsInside(const AActor* Actor, const FVector& Center, const FQuat& Rotation) const override;
};