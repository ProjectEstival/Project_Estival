#pragma once

#include "CoreMinimal.h"
#include "Systems/Mike/Combat/Abilities/Effects/Targeting/Abstract/TargetShape.h"
#include "BoxTargetShape.generated.h"

/** A box. Extend Forward puts it in front of the origin instead of around it. */
UCLASS(meta = (DisplayName = "Box"))
class PROJECTESTIVAL_API UBoxTargetShape : public UTargetShape
{
	GENERATED_BODY()
	
public:
	virtual void DrawDebug(const UWorld* World, const FVector& Center, const FQuat& Rotation, float Duration) const override
	{
		DrawDebugBox(World, Center + Rotation.RotateVector(GetLocalOffset()), Size * 0.5f, Rotation, FColor::Red, false, Duration);
	}
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape", meta = (Units = "cm"))
	FVector Size = FVector(150, 100, 200);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shape")
	bool bExtendForward = true;
	
	virtual FCollisionShape GetCollisionShape() const override
	{
		return FCollisionShape::MakeBox(Size * 0.5f);
	}
	virtual FVector GetLocalOffset() const override
	{
		return bExtendForward ? FVector(Size.X * 0.5f, 0.f, 0.f) : FVector::ZeroVector;
	}
};